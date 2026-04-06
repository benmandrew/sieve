import os

project = "sieve"
author = "Ben M. Andrew"

extensions = [
    "breathe",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "furo"

_doxygen_xml = os.environ.get("SPHINX_DOXYGEN_XML_DIR", "")
if _doxygen_xml:
    breathe_projects = {"sieve": _doxygen_xml}
else:
    breathe_projects = {}

breathe_default_project = "sieve"
breathe_domain_by_extension = {
    "h": "cpp",
    "hpp": "cpp",
    "cpp": "cpp",
}
