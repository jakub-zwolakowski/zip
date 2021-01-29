#! /usr/bin/env python3

# This script regenerates TrustInSoft CI configuration.

# Run from the root of the project:
# $ python3 trustinsoft/regenerate.py

import tis

import re # sub
import json # dumps, load
import os # makedirs
from os import path # path.basename, path.isdir, path.join
import glob # iglob
from itertools import product  # Cartesian product of lists.
import shutil # copyfileobj
import argparse # ArgumentParser, add_argument, parse_args

# --------------------------------------------------------------------------- #
# ----------------------------- PARSE ARGUMENTS ----------------------------- #
# --------------------------------------------------------------------------- #

parser = argparse.ArgumentParser(
    description="Regenerate the TrustInSoft CI files.",
    epilog="Please call this script only after building jansson.")
args = parser.parse_args()

# --------------------------------------------------------------------------- #
# -------------------------------- SETTINGS --------------------------------- #
# --------------------------------------------------------------------------- #

# Directories.
common_config_path = path.join("trustinsoft", "common.config")

# Architectures.
machdeps = [
    {
        "machdep": "gcc_x86_32",
        "pretty_name": "little endian 32-bit (x86)",
        "fields": {
            "address-alignment": 32,
            "cpp-extra-args": [
                "-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0"
            ],
        }
    },
    {
        "machdep": "gcc_x86_64",
        "pretty_name": "little endian 64-bit (x86)",
        "fields": {
            "address-alignment": 64
        }
    },
    {
        "machdep": "gcc_ppc_32",
        "pretty_name": "big endian 32-bit (PPC32)",
        "fields": {
            "address-alignment": 32,
            "cpp-extra-args": [
                "-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
            ],
        },
    },
    {
        "machdep": "gcc_ppc_64",
        "pretty_name": "big endian 64-bit (PPC64)",
        "fields": {
            "address-alignment": 64,
            "cpp-extra-args": [
                "-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
            ],
        },
    },
]

# --------------------------------------------------------------------------- #
# ---------------------------------- CHECKS --------------------------------- #
# --------------------------------------------------------------------------- #

# Initial check.
print("1. Check if all necessary directories and files exist...")
tis.check_dir("trustinsoft")

# --------------------------------------------------------------------------- #
# -------------------- GENERATE trustinsoft/common.config ------------------- #
# --------------------------------------------------------------------------- #

def make_common_config():
    return {
        "prefix_path": "..",
        "files": [
            "trustinsoft/stub.c",
            "src/zip.c"
        ],
        "machdep": "gcc_x86_64",
        "cpp-extra-args": [
            "-Isrc",
            "-DMINIZ_NO_TIME"
        ],
        "filesystem": {
            "system_errors": False,
            "files": [
                {
                    "name": "/dev/null"
                },
                {
                    "name": "./dotfiles",
                    "type": "dir"
                },
                {
                    "name": "/tmp",
                    "type": "dir"
                },
                {
                    "name": "foo.zip",
                    "from": "trustinsoft/inputs/foo.zip"
                },
                {
                    "name": "foo-2.1.txt",
                    "from": "trustinsoft/inputs/foo-2.1.txt"
                },
                {
                    "name": "foo-2.2.txt",
                    "from": "trustinsoft/inputs/foo-2.2.txt"
                },
                {
                    "name": "foo-2.3.txt",
                    "from": "trustinsoft/inputs/foo-2.3.txt"
                },
                {
                    "name": "compress_me/I_want_to_be_compressed.txt",
                    "from": "trustinsoft/inputs/foo-2.1.txt"
                },
                {
                    "name": "compress_me/compression_is_my_destiny.txt",
                    "from": "trustinsoft/inputs/foo-2.2.txt"
                },
                {
                    "name": "compress_me/compress_me_too/I_am_the_biggest_fan_of_compression.txt",
                    "from": "trustinsoft/inputs/foo-2.3.txt"
                }
            ]
        }
    }

common_config = make_common_config()
with open(common_config_path, "w") as file:
    print("3. Generate the '%s' file." % common_config_path)
    file.write(tis.string_of_json(common_config))

# ---------------------------------------------------------------------------- #
# ------------------ GENERATE trustinsoft/<machdep>.config ------------------- #
# ---------------------------------------------------------------------------- #

def make_machdep_config(machdep):
    machdep_config = {
        "machdep": machdep["machdep"]
    }
    fields = machdep["fields"]
    for field in fields:
        machdep_config[field] = fields[field]
    return machdep_config

print("4. Generate 'trustinsoft/<machdep>.config' files...")
machdep_configs = map(make_machdep_config, machdeps)
for machdep_config in machdep_configs:
    file = path.join("trustinsoft", "%s.config" % machdep_config["machdep"])
    with open(file, "w") as f:
        print("   > Generate the '%s' file." % file)
        f.write(tis.string_of_json(machdep_config))

# --------------------------------------------------------------------------- #
# --------------------------- GENERATE tis.config --------------------------- #
# --------------------------------------------------------------------------- #

def make_main_test(machdep):
    return {
        "name": "test.c, %s" % (machdep["pretty_name"]),
        "include": common_config_path,
        "include_": path.join("trustinsoft", "%s.config" % machdep["machdep"]),
        "files": [ path.join("test", "test.c") ],
    }

example_functions = [
    "create",
    "append",
    "extract_into_a_folder",
    "extract_into_memory",
    "extract_into_memory_no_allocation",
    "extract_entry_into_a_file",
    "list_all_entries",
    "compress_folder_recursively",
]

def make_example_test(main_function, machdep):
    return {
        "name": "examples.c : %s(), %s" % (main_function, machdep["pretty_name"]),
        "include": common_config_path,
        "include_": path.join("trustinsoft", "%s.config" % machdep["machdep"]),
        "files": [ path.join("trustinsoft", "examples.c") ],
        "main": main_function,
    }

def make_tis_config():
    return (
        list(map(make_main_test, machdeps)) +
        list(map(
            lambda t: make_example_test(t[0], t[1]),
            product(example_functions, machdeps)
        ))
    )

tis_config = make_tis_config()
with open("tis.config", "w") as file:
    print("5. Generate the 'tis.config' file.")
    file.write(tis.string_of_json(tis_config))
