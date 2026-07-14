#!/usr/bin/env python3

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class PerformanceBaselineTest(unittest.TestCase):
    def test_repeated_reports_are_aggregated_with_reproducibility_metadata(self):
        repository = Path(__file__).resolve().parents[1]
        runner = repository / "python" / "run_performance_baseline.py"
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            fixture = directory / "input.dat"
            fixture.write_bytes(b"fixture")
            workload = directory / "workload.py"
            workload.write_text(
                "import json, os\n"
                "json.dump({\"wall_seconds\": 2.0, \"cpu_seconds\": 1.0, "
                "\"events_per_second\": 5.0, \"cpu_seconds_per_event\": 0.1, "
                "\"peak_rss_kib\": 32, \"counters\": {\"events\": 10}, "
                "\"phases\": {\"selection\": {\"seconds\": 0.5, "
                "\"calls\": 10}}}, open(os.environ[\"SKNANO_PERFORMANCE_REPORT\"], \"w\"))\n"
            )
            output = directory / "baseline.json"
            subprocess.run(
                [sys.executable, str(runner), "--workload", "unit",
                 "--output", str(output), "--runs", "3", "--cache-mode",
                 "warm", "--warmup-runs", "1", "--input", str(fixture),
                 "--", sys.executable,
                 str(workload)],
                check=True, capture_output=True, text=True)
            artifact = json.loads(output.read_text())
            self.assertEqual(artifact["runs"], 3)
            self.assertEqual(artifact["warmup_runs"], 1)
            self.assertEqual(len(artifact["warmup_logs"]), 1)
            self.assertEqual(artifact["aggregate"]["wall_seconds"]["median"], 2.0)
            self.assertEqual(
                artifact["aggregate"]["events_per_second"]["median"], 5.0)
            self.assertEqual(
                artifact["aggregate"]["phase.selection.calls"]["mad"], 0.0)
            self.assertEqual(
                artifact["inputs"][0]["sha256"],
                hashlib.sha256(b"fixture").hexdigest())
            self.assertEqual(len(artifact["reports"]), 3)


if __name__ == "__main__":
    unittest.main()
