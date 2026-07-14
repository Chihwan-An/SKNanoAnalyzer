#!/usr/bin/env python3
"""Unit tests for the deterministic, ROOT-independent schema merge contract."""

import importlib.util
import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "python" / "dump_branch_schema.py"
SPEC = importlib.util.spec_from_file_location("dump_branch_schema", MODULE_PATH)
assert SPEC and SPEC.loader
SCHEMA = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SCHEMA)

GENERATOR_PATH = Path(__file__).resolve().parents[1] / "python" / "generate_branch_code.py"
GENERATOR_SPEC = importlib.util.spec_from_file_location("generate_branch_code", GENERATOR_PATH)
assert GENERATOR_SPEC and GENERATOR_SPEC.loader
GENERATOR = importlib.util.module_from_spec(GENERATOR_SPEC)
GENERATOR_SPEC.loader.exec_module(GENERATOR)


def branch(name, leaf_type, cardinality):
    return {"name": name, "leaf_type": leaf_type, "cardinality": cardinality}


def file_schema(path, branches):
    return {"path": path, "tree": "Events", "branches": branches}


class SchemaPipelineTest(unittest.TestCase):
    def test_run_cpp_alias_preserves_physical_branch_name_and_input(self):
        events = {"run": {"run": {"type": "UInt_t"}}}
        before = json.loads(json.dumps(events))
        declarations, registrations, resets = GENERATOR.build_blocks(events)
        self.assertEqual(events, before)
        self.assertIn('BranchScalar<unsigned int> RunNumber{"run"};',
                      [line.strip() for line in declarations])
        self.assertIn("branchManager.registerScalar(RunNumber);",
                      [line.strip() for line in registrations])
        self.assertIn("branchManager.reset(RunNumber);",
                      [line.strip() for line in resets])

    def test_view_codegen_emits_composition_bind_and_metadata(self):
        events = {
            "Jet": {
                "nJet": {"type": "Int_t"},
                "Jet_pt": {"type": "Float_t"},
                "Jet_cleanMask": {"type": "Bool_t"},
            }
        }
        overlay = {
            "view_collections": {
                "Jet": {
                    "required": True,
                    "fields": {"Jet_cleanMask": {"required": False}},
                }
            }
        }
        output = GENERATOR.build_view_blocks(events, overlay)
        stripped = [line.strip() for line in output]
        self.assertIn("struct JetInputColumnsGenerated {", stripped)
        self.assertIn("ColumnView<float> pt;", stripped)
        self.assertIn("BoolColumnView cleanMask;", stripped)
        self.assertIn("static constexpr bool cleanMaskRequired = false;", stripped)
        self.assertIn("bool cleanMaskAvailable() const { return cleanMask.branch() && cleanMask.branch()->valid(); }", stripped)
        self.assertIn("columns.pt.bind(&Jet_pt);", stripped)
        self.assertIn("JetInputColumnsGenerated GetJetInputColumns() {", stripped)

    def test_addon_codegen_merges_without_mutating_base_and_emits_lazy_rows(self):
        events = {
            "GenJet": {
                "nGenJet": {"type": "Int_t"},
                "GenJet_pt": {"type": "Float_t"},
            }
        }
        before = json.loads(json.dumps(events))
        addon = {
            "name": "FixtureAddon",
            "collections": {
                "GenJet": {
                    "count": {"name": "nGenJet", "type": "Int_t"},
                    "presence_branch": "GenJet_nD0",
                    "view_name": "GenJetFixture",
                    "fields": {"GenJet_nD0": "UChar_t"},
                },
                "Candidate": {
                    "count": {"name": "nCandidate", "type": "Int_t"},
                    "presence_branch": "nCandidate",
                    "fields": {
                        "Candidate_pt": "Float_t",
                        "Candidate_selected": "Bool_t",
                    },
                },
            },
        }
        merged, specs = GENERATOR.merge_addons(events, [addon])
        self.assertEqual(events, before)
        self.assertEqual(merged["GenJet"]["GenJet_nD0"]["type"], "UChar_t")
        self.assertEqual(merged["Candidate"]["nCandidate"]["type"], "Int_t")
        output = [line.strip() for line in GENERATOR.build_addon_view_blocks(specs)]
        self.assertIn("struct CandidateViewCollectionGenerated {", output)
        self.assertIn("BoolColumnView selected;", output)
        self.assertIn("float pt() const { return columns_->pt[index_]; }", output)
        self.assertIn("CandidateViewCollectionGenerated GetAllCandidateViews() {", output)
        self.assertIn("GenJetFixtureViewCollectionGenerated GetAllGenJetFixtureViews() {", output)

    def test_addon_cannot_redefine_standard_branch_type(self):
        events = {"Jet": {"nJet": {"type": "Int_t"}, "Jet_pt": {"type": "Float_t"}}}
        addon = {
            "name": "BadAddon",
            "collections": {
                "Jet": {
                    "count": {"name": "nJet", "type": "Int_t"},
                    "fields": {"Jet_pt": "Int_t"},
                }
            },
        }
        with self.assertRaisesRegex(ValueError, "changes type of 'Jet_pt'"):
            GENERATOR.merge_addons(events, [addon])

    def test_codegen_creates_missing_output_directory(self):
        repository = Path(__file__).resolve().parents[1]
        schema = repository / "data" / "Run3_v15_Run2_v15" / "NanoAODv15.json"
        overlay = repository / "data" / "Run3_v15_Run2_v15" / "branch_overlay.json"
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "nested" / "generated"
            self.assertFalse(output.exists())
            subprocess.run(
                [
                    "python3", str(GENERATOR_PATH), "--json", str(schema),
                    "--overlay", str(overlay), "--out-dir", str(output),
                ],
                check=True, capture_output=True, text=True,
            )
            self.assertEqual(
                {path.name for path in output.iterdir()},
                {
                    "generated_branch_decls.inc",
                    "generated_branch_register.inc",
                    "generated_branch_reset.inc",
                    "generated_input_columns.inc",
                    "generated_loader_api.inc",
                },
            )

    @unittest.skipUnless(shutil.which("cmake"), "CMake is unavailable")
    def test_build_directory_codegen_second_build_is_noop(self):
        repository = Path(__file__).resolve().parents[1]
        generator = repository / "python" / "generate_branch_code.py"
        schema = repository / "data" / "Run3_v15_Run2_v15" / "NanoAODv15.json"
        overlay = repository / "data" / "Run3_v15_Run2_v15" / "branch_overlay.json"
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "source"
            build = Path(directory) / "build"
            source.mkdir()
            source.joinpath("CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.20)\n"
                "project(codegen NONE)\n"
                "find_package(Python3 REQUIRED COMPONENTS Interpreter)\n"
                "set(out ${CMAKE_BINARY_DIR}/generated)\n"
                "set(outputs ${out}/generated_branch_decls.inc "
                "${out}/generated_branch_register.inc "
                "${out}/generated_branch_reset.inc "
                "${out}/generated_input_columns.inc "
                "${out}/generated_loader_api.inc)\n"
                "add_custom_command(OUTPUT ${outputs} "
                "COMMAND ${CMAKE_COMMAND} -E make_directory ${out} "
                f"COMMAND ${{Python3_EXECUTABLE}} {generator} --json {schema} "
                f"--overlay {overlay} --out-dir ${{out}} "
                f"DEPENDS {generator} {schema} {overlay} VERBATIM)\n"
                "add_custom_target(codegen ALL DEPENDS ${outputs})\n"
            )
            subprocess.run(["cmake", "-S", str(source), "-B", str(build)],
                           check=True, capture_output=True, text=True)
            subprocess.run(["cmake", "--build", str(build)], check=True,
                           capture_output=True, text=True)
            outputs = sorted((build / "generated").iterdir())
            first_mtimes = {path.name: path.stat().st_mtime_ns for path in outputs}
            subprocess.run(["cmake", "--build", str(build)], check=True,
                           capture_output=True, text=True)
            second_mtimes = {path.name: path.stat().st_mtime_ns for path in outputs}
            self.assertEqual(first_mtimes, second_mtimes)

    def test_union_is_byte_identical_independent_of_input_order(self):
        first = file_schema(
            "/fixtures/a.root",
            [
                branch("nJet", "Int_t", {"kind": "scalar", "static_length": 1}),
                branch(
                    "Jet_pt",
                    "Float_t",
                    {"kind": "variable", "static_length": 1, "count_branch": "nJet", "count_leaf_type": "Int_t"},
                ),
                branch("fixed", "Float_t", {"kind": "fixed", "static_length": 4}),
            ],
        )
        second = file_schema(
            "/fixtures/b.root",
            [
                branch("nJet", "Int_t", {"kind": "scalar", "static_length": 1}),
                branch(
                    "Jet_pt",
                    "Float_t",
                    {"kind": "variable", "static_length": 1, "count_branch": "nJet", "count_leaf_type": "Int_t"},
                ),
                branch("run", "UInt_t", {"kind": "scalar", "static_length": 1}),
            ],
        )
        forward = SCHEMA.merge_raw_schemas([first, second], "Events")
        reverse = SCHEMA.merge_raw_schemas([second, first], "Events")
        self.assertEqual(SCHEMA.canonical_json(forward), SCHEMA.canonical_json(reverse))
        jet_pt = next(item for item in forward["branches"] if item["name"] == "Jet_pt")
        fixed = next(item for item in forward["branches"] if item["name"] == "fixed")
        self.assertTrue(jet_pt["required_in_all_inputs"])
        self.assertEqual(fixed["available_in"], [0])
        self.assertFalse(fixed["required_in_all_inputs"])
        report = SCHEMA.availability_report(forward)
        self.assertEqual(report["input_count"], 2)
        self.assertEqual(report["union_branch_count"], 4)
        self.assertEqual(report["intersection_branch_count"], 2)
        self.assertEqual(
            next(item for item in report["availability"] if item["name"] == "fixed")["available_in"],
            ["/fixtures/a.root"],
        )

    def test_conflicting_type_or_cardinality_fails(self):
        first = file_schema("/fixtures/a.root", [branch("score", "Float_t", {"kind": "scalar", "static_length": 1})])
        second = file_schema("/fixtures/b.root", [branch("score", "Int_t", {"kind": "scalar", "static_length": 1})])
        with self.assertRaisesRegex(SCHEMA.SchemaError, "schema conflict.*score"):
            SCHEMA.merge_raw_schemas([first, second], "Events")

    def test_missing_or_wrong_count_branch_fails(self):
        missing = file_schema(
            "/fixtures/missing.root",
            [branch("Jet_pt", "Float_t", {"kind": "variable", "static_length": 1, "count_branch": "nJet", "count_leaf_type": "Int_t"})],
        )
        with self.assertRaisesRegex(SCHEMA.SchemaError, "missing count branch"):
            SCHEMA.merge_raw_schemas([missing], "Events")

        wrong_type = file_schema(
            "/fixtures/wrong.root",
            [
                branch("nJet", "UInt_t", {"kind": "scalar", "static_length": 1}),
                branch("Jet_pt", "Float_t", {"kind": "variable", "static_length": 1, "count_branch": "nJet", "count_leaf_type": "Int_t"}),
            ],
        )
        with self.assertRaisesRegex(SCHEMA.SchemaError, "expects count.*Int_t.*UInt_t"):
            SCHEMA.merge_raw_schemas([wrong_type], "Events")

    def test_overlay_cannot_change_raw_facts(self):
        raw = SCHEMA.merge_raw_schemas(
            [file_schema("/fixtures/a.root", [branch("score", "Float_t", {"kind": "scalar", "static_length": 1})])],
            "Events",
        )
        merged = SCHEMA.merge_overlay(raw, {"overlay_version": 2, "overrides": {"score": {"required": True, "view_enabled": False}}, "derived": {"score2": {"expression": "score * score"}}})
        self.assertEqual(merged["branches"][0]["metadata"], {"required": True, "view_enabled": False})
        self.assertEqual(merged["derived"]["score2"]["expression"], "score * score")
        with self.assertRaisesRegex(SCHEMA.SchemaError, "cannot override raw schema fields"):
            SCHEMA.merge_overlay(raw, {"overrides": {"score": {"leaf_type": "Int_t"}}})

    @unittest.skipUnless(shutil.which("root"), "ROOT command is unavailable")
    def test_real_root_fixture_preserves_leaf_cardinality(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = Path(directory) / "schema.root"
            code = (
                'int nJet=0; float Jet_pt[8]{}; float fixedArray[4]{}; unsigned int run=0; '
                f'TFile file({json.dumps(str(fixture))}, "RECREATE"); '
                'TTree tree("Events", "Events"); '
                'tree.Branch("nJet", &nJet, "nJet/I"); '
                'tree.Branch("Jet_pt", Jet_pt, "Jet_pt[nJet]/F"); '
                'tree.Branch("fixedArray", fixedArray, "fixedArray[4]/F"); '
                'tree.Branch("run", &run, "run/i"); '
                'nJet=2; tree.Fill(); tree.Write(); file.Close();'
            )
            subprocess.run(["root", "-b", "-q", "-e", code], check=True, capture_output=True, text=True)
            raw = SCHEMA.extract_file_schema(fixture, "Events")
            by_name = {item["name"]: item for item in raw["branches"]}
            self.assertEqual(by_name["run"]["leaf_type"], "UInt_t")
            self.assertEqual(by_name["fixedArray"]["cardinality"], {"kind": "fixed", "static_length": 4})
            self.assertEqual(
                by_name["Jet_pt"]["cardinality"],
                {"kind": "variable", "static_length": 1, "count_branch": "nJet", "count_leaf_type": "Int_t"},
            )


if __name__ == "__main__":
    unittest.main()
