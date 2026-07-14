import tempfile
import unittest
from pathlib import Path

from python.sample_paths import resolve_sample_paths


class SamplePathTest(unittest.TestCase):
    def test_legacy_explicit_paths_preserve_order(self):
        self.assertEqual(
            resolve_sample_paths({"path": ["b.root", "a.root"]}),
            ["b.root", "a.root"],
        )

    def test_recursive_glob_is_sorted_and_root_only(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "shard").mkdir()
            (root / "shard" / "b.root").touch()
            (root / "a.root").touch()
            (root / "ignore.txt").touch()
            paths = resolve_sample_paths({"path_glob": f"{root}/**/*"})
            self.assertEqual(paths, sorted([
                str(root / "a.root"),
                str(root / "shard" / "b.root"),
            ]))

    def test_empty_glob_fails_clearly(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, "matched no ROOT files"):
                resolve_sample_paths({"path_glob": f"{directory}/**/*.root"})


if __name__ == "__main__":
    unittest.main()
