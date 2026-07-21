"""Schema-driven ROOT histogram plotting for SKNano outputs."""

from .catalog import Catalog, CatalogEntry, inspect_root
from .schema import PlotSchema, SchemaError, load_schema, validate_schema

__all__ = [
    "Catalog",
    "CatalogEntry",
    "PlotSchema",
    "SchemaError",
    "inspect_root",
    "load_schema",
    "validate_schema",
]

__version__ = "1.0.0"
