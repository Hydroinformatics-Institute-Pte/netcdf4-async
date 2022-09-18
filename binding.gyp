{
    "targets": [
        {
            "libraries": [
                "-lnetcdf"
            ],
            'cflags_cc': [
                '-Wall',
                '-Wextra',
                '--pedantic-errors',
                '-Wfloat-equal',
                '-Wuninitialized',
                '-Wunreachable-code',
                '-Wold-style-cast',
                '-Werror',
                '-fexceptions',
                '-std=c++14'
            ],
            "sources": [
                "src/netcdf4-async.cpp",
                "src/async.cpp",
                "src/Macros.cpp",
                "src/File.cpp",
                "src/Attribute.cpp",
                "src/Group.cpp",
                "src/Variable.cpp"
                
            ],
            "target_name": "netcdf4-async",
            "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
            "target_name": "netcdf4-async",
            'defines': ['NAPI_CPP_EXCEPTIONS'],
            "conditions": [
                ['OS=="mac"', {
                    'cflags+': ['-fvisibility=hidden','-std=c++14'],
                    'xcode_settings': {
                        'WARNING_CFLAGS!': ['-Wall', '-Wextra'],
                        'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',  # -fvisibility=hidden
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'CLANG_CXX_LIBRARY': 'libc++',
                        'MACOSX_DEPLOYMENT_TARGET': '10.15',
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++14']                        
                    }
                }],
                ['OS=="win"', {
                    "variables": {
                        "netcdf_dir%": "<!(echo %NETCDF_DIR%)"
                    },
                    "include_dirs": [
                        "<(netcdf_dir)/include"
                    ],
                    "msvs_settings": {
                        'VCCLCompilerTool': {
                            'WarningLevel': '3',
                            'WarnAsError': 'false',
                            'ExceptionHandling': 1
                        },
                        "VCLinkerTool": {
                            "AdditionalLibraryDirectories": "<(netcdf_dir)/lib"
                        }
                    }
                }]
            ]
        }
    ]
}
