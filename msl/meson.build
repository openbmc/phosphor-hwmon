if get_option('enable-max31785-msl')
    configure_file(
        copy: true,
        input: 'max31785-msl',
        install: true,
        install_dir: get_option('bindir'),
        install_mode: 'rwxr-xr-x',
        output: 'max31785-msl',
    )

    install_data(
        'phosphor-max31785-msl@.service',
        install_dir: systemd_system_unit_dir,
    )
endif
