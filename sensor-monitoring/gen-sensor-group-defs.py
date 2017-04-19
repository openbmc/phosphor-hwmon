#!/usr/bin/env python

import os
import sys
import yaml
from argparse import ArgumentParser
from mako.template import Template


def get_file(dir_path, filename):
    return os.path.join(dir_path, filename)


def get_value_init(value_type):
    return {
        'int64_t': 0,
        'bool': "false",
        'string': ""
    }[value_type]


def get_condition_type(conditions, condition):
    for c in conditions:
        # Return the first instance of the condition found for its type
        if c['name'] == condition:
            return c['value']['type']


def set_data_type(groups, egroup, value_type, value_init):
    for g in groups:
        if g['name'] in egroup:
            g['value_type'] = value_type
            g['value_init'] = value_init


def set_group_data_type(yaml_data):
    for e in yaml_data['events']:
        s = set()
        value_type = None
        value_init = None
        for t in e['triggers']:
            for c in t['conditions']:
                s.add(c)
        for condition in s:
            value_type = get_condition_type(yaml_data['conditions'], condition)
            value_init = get_value_init(value_type)
        set_data_type(yaml_data['groups'], e['groups'], value_type, value_init)


def generate(yaml_file, output_file):
    with open(yaml_file, 'r') as yaml_input:
        yaml_data = yaml.safe_load(yaml_input) or {}

    set_group_data_type(yaml_data)

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
