import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "python"))

import JobReporter
import telegram_reporter


class TelegramClientTest(unittest.TestCase):
    def test_strip_emoji(self):
        self.assertEqual(
            telegram_reporter._strip_emoji("🚀 submitted ✅ failed ❌ warning ⚠️"),
            " submitted  failed  warning ",
        )

    def test_submission_message_has_no_emoji(self):
        message = telegram_reporter.submission_message({})
        self.assertEqual(message.splitlines()[0], "SKNano job submitted")
        self.assertEqual(telegram_reporter._strip_emoji(message), message)

    def test_credentials_fall_back_to_config(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            config = Path(tmpdir) / "config.test"
            config.write_text(
                "[TOKEN_TELEGRAMBOT] secret-token\n[USER_CHATID] 12345\n",
                encoding="utf-8",
            )
            with mock.patch.dict(os.environ, {}, clear=True):
                self.assertEqual(
                    telegram_reporter.load_telegram_credentials(str(config)),
                    ("secret-token", "12345"),
                )

    def test_send_uses_post_and_splits_long_messages(self):
        response = mock.Mock()
        response.raise_for_status.return_value = None
        response.json.return_value = {"ok": True}
        with mock.patch.object(
            telegram_reporter, "load_telegram_credentials", return_value=("token", "chat")
        ), mock.patch.object(telegram_reporter.requests, "post", return_value=response) as post:
            self.assertTrue(telegram_reporter.send_telegram_message("x" * 5000))
            self.assertEqual(post.call_count, 2)
            self.assertEqual(post.call_args.kwargs["timeout"], (5.0, 15.0))
            self.assertNotIn("token", post.call_args.kwargs["data"])

    def test_send_redacts_token_from_transport_errors(self):
        error = telegram_reporter.requests.RequestException(
            "request failed for https://api.telegram.org/botsecret-token/sendMessage"
        )
        with mock.patch.object(
            telegram_reporter,
            "load_telegram_credentials",
            return_value=("secret-token", "chat"),
        ), mock.patch.object(telegram_reporter.requests, "post", side_effect=error), mock.patch(
            "sys.stderr"
        ) as stderr:
            self.assertFalse(telegram_reporter.send_telegram_message("hello"))
            logged = "".join(call.args[0] for call in stderr.write.call_args_list if call.args)
            self.assertNotIn("secret-token", logged)
            self.assertIn("[REDACTED]", logged)


class FinalReportTest(unittest.TestCase):
    def _write_manifest(self, root: Path, workdir: Path, output: Path):
        manifest = {
            "created_at": "2026-07-14T10:00:00",
            "master_dir": str(root),
            "options": {"Analyzer": "DiLepton", "Memory": 2048},
            "samples": [
                {
                    "era": "2022EE",
                    "sample": "DYJets",
                    "jobs": 3,
                    "working_dir": str(workdir),
                    "output": str(output),
                }
            ],
            "submit": {"dag_file": str(root / "dags" / "finaldag.dag")},
        }
        (root / "run_manifest.json").write_text(json.dumps(manifest), encoding="utf-8")

    def test_failed_report_classifies_error_and_includes_retry(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            dag_dir = root / "dags"
            workdir = root / "2022EE" / "DYJets"
            dag_dir.mkdir()
            workdir.mkdir(parents=True)
            self._write_manifest(root, workdir, root / "output" / "DYJets.root")
            (dag_dir / "dagfile.dag.dagman.out").write_text(
                "DAG status: 1 (DAG_STATUS_ERROR)\nJob was held\nMemoryUsage = 2200\n",
                encoding="utf-8",
            )
            (dag_dir / "dagfile.dag.rescue001").write_text(
                "# Total number of Nodes: 4\n"
                "# Nodes that failed: 1\n"
                "# Analyzer_DiLepton_2022EE_DYJets:2,ENDLIST\n",
                encoding="utf-8",
            )
            (workdir / "job_2.err").write_text(
                "terminate called after throwing std::bad_alloc\n", encoding="utf-8"
            )

            report = JobReporter.build_report(root)
            self.assertIn("SKNano job failed", report)
            self.assertIn("Memory limit / OOM: 1 file(s)", report)
            self.assertIn("Failed DAG nodes: 1", report)
            self.assertIn("condor_submit_dag -force", report)
            self.assertIn("Held events: 1", report)

    def test_successful_retry_ignores_stale_rescue(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            dag_dir = root / "dags"
            workdir = root / "2022EE" / "DYJets"
            output = root / "output" / "DYJets.root"
            dag_dir.mkdir()
            workdir.mkdir(parents=True)
            output.parent.mkdir()
            output.touch()
            self._write_manifest(root, workdir, output)
            (dag_dir / "dagfile.dag.dagman.out").write_text(
                "DAG status: 0 (DAG_STATUS_OK)\n", encoding="utf-8"
            )
            (dag_dir / "dagfile.dag.rescue001").write_text(
                "# Total number of Nodes: 4\n"
                "# Nodes that failed: 1\n"
                "# old_failed_node,ENDLIST\n",
                encoding="utf-8",
            )

            report = JobReporter.build_report(root)
            self.assertIn("SKNano job completed", report)
            self.assertNotIn("Failed DAG nodes", report)
            self.assertNotIn("Retry from", report)

    def test_outer_dag_removal_takes_precedence(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            dag_dir = Path(tmpdir)
            (dag_dir / "dagfile.dag.dagman.out").write_text(
                "DAG status: 0 (DAG_STATUS_OK)\n", encoding="utf-8"
            )
            (dag_dir / "finaldag.dag.dagman.out").write_text(
                "DAG status: 4 (DAG_STATUS_RM)\n", encoding="utf-8"
            )
            self.assertEqual(JobReporter.read_dag_status(dag_dir), (4, "DAG_STATUS_RM"))


if __name__ == "__main__":
    unittest.main()
