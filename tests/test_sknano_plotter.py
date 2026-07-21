import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
import yaml


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "python"))

from sknano_plotter.catalog import inspect_root
from sknano_plotter.draft import build_draft
from sknano_plotter.histogram import Histogram1D, rebin1d, total_uncertainty
from sknano_plotter.schema import load_schema, validate_schema
from sknano_plotter.tasks import build_tasks
from sknano_plotter.render import render_task


class PlotterSchemaTest(unittest.TestCase):
    def setUp(self):
        try:
            import uproot
        except ModuleNotFoundError:
            self.skipTest("uproot is not installed")
        self.uproot = uproot

    def _fixture(self, directory: str) -> Path:
        path = Path(directory) / "fixture.root"
        edges = np.array([0.0, 1.0, 2.0, 3.0, 4.0])
        with self.uproot.recreate(path) as root_file:
            root_file["Analysis/A/h"] = (np.array([1.0, 2.0, 3.0, 4.0]), edges)
            root_file["Analysis/B/h"] = (np.array([2.0, 3.0, 4.0, 5.0]), edges)
            root_file["Analysis/Deep/X/map"] = (
                np.arange(6, dtype=float).reshape(2, 3),
                np.array([0.0, 1.0, 2.0]),
                np.array([0.0, 1.0, 2.0, 3.0]),
            )
        return path

    def _schema(self, directory: str, status: str = "confirmed") -> Path:
        path = Path(directory) / "schema.yaml"
        payload = {
            "version": 1,
            "status": status,
            "metadata": {},
            "tree": {
                "rules": [
                    {
                        "name": "shapes",
                        "kinds": ["TH1"],
                        "pattern": r"Analysis/(?P<category>A|B)/(?P<observable>[^/]+)",
                    },
                    {
                        "name": "maps",
                        "kinds": ["TH2"],
                        "pattern": r"Analysis/Deep/(?P<category>[^/]+)/(?P<observable>[^/]+)",
                    },
                ]
            },
            "recipes": [
                {
                    "name": "compare",
                    "renderer": "overlay",
                    "source": "shapes",
                    "enabled": True,
                    "facets": ["observable"],
                    "series": "category",
                },
                {
                    "name": "maps",
                    "renderer": "heatmap",
                    "source": "maps",
                    "enabled": True,
                },
            ],
        }
        path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")
        return path

    def test_catalog_and_variable_depth_schema(self):
        with tempfile.TemporaryDirectory() as directory:
            catalog = inspect_root(self._fixture(directory))
            self.assertEqual(catalog.counts["TH1"], 2)
            self.assertEqual(catalog.counts["TH2"], 1)
            report = validate_schema(catalog, load_schema(self._schema(directory)), require_confirmed=True)
            self.assertTrue(report.ok, report.errors)
            tasks = build_tasks(load_schema(self._schema(directory)), report)
            self.assertEqual([task["renderer"] for task in tasks], ["overlay", "heatmap"])
            self.assertEqual(len(tasks[0]["members"]), 2)

    def test_draft_is_conservative_and_unconfirmed(self):
        with tempfile.TemporaryDirectory() as directory:
            catalog = inspect_root(self._fixture(directory))
            draft = build_draft(catalog)
            self.assertEqual(draft["status"], "draft")
            self.assertTrue(draft["recipes"])
            self.assertTrue(all(not recipe["enabled"] for recipe in draft["recipes"]))

    def test_uncovered_and_duplicate_paths_fail_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            catalog = inspect_root(self._fixture(directory))
            schema_path = self._schema(directory)
            raw = yaml.safe_load(schema_path.read_text())
            raw["tree"]["rules"].append(
                {"name": "duplicate", "kinds": ["TH1"], "pattern": r"Analysis/.+"}
            )
            schema_path.write_text(yaml.safe_dump(raw, sort_keys=False))
            report = validate_schema(catalog, load_schema(schema_path))
            self.assertFalse(report.ok)
            self.assertEqual(len(report.duplicates), 2)


class HistogramMathTest(unittest.TestCase):
    def _hist(self, values, variances=None):
        values = np.asarray(values, dtype=float)
        if variances is None:
            variances = values
        return Histogram1D(values, np.asarray(variances, dtype=float), np.arange(len(values) + 1, dtype=float))

    def test_rebin_preserves_yield_and_variance(self):
        histogram = self._hist([1, 2, 3, 4], [1, 4, 9, 16])
        rebinned = rebin1d(histogram, 2)
        np.testing.assert_allclose(rebinned.values, [3, 7])
        np.testing.assert_allclose(rebinned.variances, [5, 25])
        with self.assertRaisesRegex(ValueError, "without discarding"):
            rebin1d(self._hist([1, 2, 3]), 2)

    def test_directional_total_uncertainty_and_missing_policy(self):
        central = {"A": self._hist([10, 20], [1, 4]), "B": self._hist([5, 10], [1, 1])}
        variations = {
            "A": {"JES_Up": self._hist([12, 18]), "JES_Down": self._hist([9, 24])},
            "B": {"JES_Up": self._hist([6, 11]), "JES_Down": self._hist([4, 9])},
        }
        low, high, warnings = total_uncertainty(central, variations)
        np.testing.assert_allclose(high, np.sqrt(np.array([2, 5]) + np.array([3, 3]) ** 2))
        np.testing.assert_allclose(low, np.sqrt(np.array([2, 5]) + np.array([2, 1]) ** 2))
        self.assertEqual(warnings, [])
        del variations["B"]["JES_Down"]
        with self.assertRaisesRegex(ValueError, "missing variation"):
            total_uncertainty(central, variations)
        _, _, warnings = total_uncertainty(central, variations, missing="central")
        self.assertTrue(warnings)


class RealHadronSchemaSmokeTest(unittest.TestCase):
    def test_example_schema_covers_current_hadron_output(self):
        root_file = REPO_ROOT / "SKNanoOutput/HadronAnalyzer/Mu/2024/TTLJ_powheg_CustomBPH.root"
        if not root_file.exists():
            self.skipTest("local HadronAnalyzer output is unavailable")
        catalog = inspect_root(root_file)
        if not catalog.entries:
            self.skipTest("local HadronAnalyzer output contains no histograms")
        schema = load_schema(REPO_ROOT / "python/sknano_plotter/examples/nested_validation.yaml")
        report = validate_schema(catalog, schema, require_confirmed=True)
        self.assertTrue(report.ok, report.errors + report.uncovered[:5])
        tasks = build_tasks(schema, report)
        rendered_paths = {
            member["path"] for task in tasks for member in task["members"]
        }
        self.assertEqual(rendered_paths, {entry.path for entry in catalog.entries})


class RendererSmokeTest(unittest.TestCase):
    def test_data_stack_ratio_and_total_band(self):
        import importlib.util

        if importlib.util.find_spec("matplotlib") is None or importlib.util.find_spec("mplhep") is None:
            self.skipTest("plotting dependencies are not installed")
        import uproot
        with tempfile.TemporaryDirectory() as directory:
            root_path = Path(directory) / "datamc.root"
            edges = np.array([0.0, 1.0, 2.0])
            with uproot.recreate(root_path) as root_file:
                for variation, values in {
                    "Central": [10.0, 20.0],
                    "JES_Up": [11.0, 22.0],
                    "JES_Down": [9.0, 18.0],
                }.items():
                    root_file[f"Ch/R/{variation}/background/x"] = (np.asarray(values), edges)
                root_file["Ch/R/Central/data_obs/x"] = (np.array([12.0, 19.0]), edges)
            task = {
                "task_id": "000000-smoke",
                "recipe": "smoke",
                "renderer": "data_stack",
                "facets": {"observable": "x"},
                "series": "process",
                "roles": {"data": "data_obs"},
                "normalize": "none",
                "rebin": 1,
                "flow": "drop",
                "log": False,
                "ratio": True,
                "ratio_range": [0.5, 1.5],
                "systematics": {
                    "dimension": "variation",
                    "nominal": "Central",
                    "up_suffix": "_Up",
                    "down_suffix": "_Down",
                    "missing": "error",
                },
                "style": {},
                "output": "smoke",
                "members": [],
            }
            with uproot.open(root_path) as root_file:
                for path, classname in root_file.classnames().items():
                    path = path.rsplit(";", 1)[0]
                    if not classname.startswith("TH1"):
                        continue
                    channel, region, variation, process, observable = path.split("/")
                    obj = root_file[path]
                    task["members"].append(
                        {
                            "path": path,
                            "classname": classname,
                            "ndim": 1,
                            "title": "",
                            "dimensions": {
                                "channel": channel,
                                "region": region,
                                "variation": variation,
                                "process": process,
                                "observable": observable,
                            },
                            "axes": [{"title": "x", "edges": list(edges), "labels": None}],
                        }
                    )
            record = render_task(str(root_path), task, directory, ["png", "pdf"], {"cms_label": "Preliminary"}, {})
            self.assertTrue(Path(record["files"]["png"]).exists())
            self.assertTrue(Path(record["files"]["pdf"]).exists())
            self.assertEqual(record["warnings"], [])


if __name__ == "__main__":
    unittest.main()
