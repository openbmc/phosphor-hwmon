if get_option('enable-max31785-msl')
configure_file(
    copy: true,
    input: 'max31785-msl',
    install: true,
    install_dir: get_option('bindir'),
    output: 'max31785-msl',
)

systemd_system_unit_dir = dependency('systemd').get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])
install_data(
    'phosphor-max31785-msl@.service',
    install_dir: systemd_system_unit_dir
)
endif
