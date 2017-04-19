#!/usr/bin/env python

import os
import sys
import yaml
from argparse import ArgumentParser
from mako.template import Template


def get_value_type(value_type):
    return {
        'int64': "int64_t"
    }.get(value_type)


def get_value_init(value_type):
    return {
        'int64': 0
    }.get(value_type)


def get_condition_type(conditions, condition):
    for c in conditions:
        if c['name'] == condition:
            return c['parameters']['value']['type']
    return None


def set_group_value_data(groups, egroups, value_type):
    for g in groups:
        if g['name'] in egroups:
            g['value_type'] = get_value_type(value_type)
            g['value_init'] = get_value_init(value_type)


def add_group_value(yaml_data):
    for e in yaml_data['events']:
        condition = lambda c: c if c else None
        set_group_value_data(yaml_data['groups'],
                             e['groups'],
                             get_condition_type(
                                 yaml_data['conditions'],
                                 condition(e['triggers'][0]['conditions'][0])))


def generate(yaml_file, output_file):
    with open(yaml_file, 'r') as yaml_input:
        yaml_data = yaml.safe_load(yaml_input) or {}

    add_group_value(yaml_data)

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
                        default=os.path.abspath('.'),
                        help=
                        "Output directory for source files generated")
    args = parser.parse_args(sys.argv[1:])

    yaml_file = os.path.abspath(args.input_yaml)
    generate(yaml_file, os.path.join(args.output_dir, "generated.cpp"))
