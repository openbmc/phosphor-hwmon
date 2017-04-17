#!/usr/bin/env python

import os
import sys
import yaml
from argparse import ArgumentParser
from mako.template import Template


def get_file(dir_path, filename):
    return os.path.join(dir_path, filename)


def generate(yaml_file, output_file):
    with open(yaml_file, 'r') as yaml_input:
        yaml_data = yaml.safe_load(yaml_input) or {}

    with open(output_file, 'w') as gen_out:
        gen_out.write(Template(filename='generated.mako.cpp').render(
            events=yaml_data))


if __name__ == '__main__':
    parser = ArgumentParser()
    # Groups of sensors and how they should be monitored yaml file
    parser.add_argument("-y", "--yaml", dest="input_yaml",
                        default=
                        "example/sensor_monitoring_defs.yaml",
                        help=
                        "Input sensor monitoring definition yaml to parse")
    parser.add_argument("-o", "--outdir", dest="output_dir",
                        default=
                        os.path.abspath('.'),
                        help=
                        "Output directory for source files generated")
    args = parser.parse_args(sys.argv[1:])

    # Verify given yaml file exists
    yaml_file = os.path.abspath(args.input_yaml)
    if not os.path.isfile(yaml_file):
        print "Unable to find input yaml file " + yaml_file
        exit(1)

    generate(yaml_file, get_file(args.output_dir, "generated.cpp"))
