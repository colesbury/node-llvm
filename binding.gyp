{
  'variables': {
    "llvm_config": "llvm-config",
  },
  'targets': [
    {
      'target_name': 'llvm',
      'sources': [
        './src/llvm.cc',
        './src/basicblock.cc',
        './src/context.cc',
        './src/irbuilder.cc',
        './src/module.cc',
        './src/value.cc',
        './src/function.cc',
        './src/type.cc',
        './src/executionengine.cc',
        './src/functionpassmanager.cc',
        './src/instructions.cc',
        './src/globals.cc',
        './src/constants.cc',
      ],
      'cflags': [
        '-O3',
        '-Wall',
        '-Werror',
      ],
      'cflags_cc': [
        '-O3',
        '-Wall',
        '-Werror',
        '-std=c++0x',
        '-g',
      ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++','-mmacosx-version-min=10.7'],
        'OTHER_LDFLAGS': ['-std=c++11'],
      },
      'defines': [
        "__STDC_LIMIT_MACROS=1",
        "__STDC_CONSTANT_MACROS=1"
      ],
      'include_dirs+': [
        "src/",
        "<!@(<(llvm_config) --includedir)"
      ],
      "libraries": [
        "<!@(<(llvm_config) --ldflags)",
        "<!@(<(llvm_config) --libs all)"
      ]
    }
  ]
}
