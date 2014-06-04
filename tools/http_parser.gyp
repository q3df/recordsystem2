# This file is used with the GYP meta build system.
# http://code.google.com/p/gyp/
{
  'targets': [
    {
      'target_name': 'http_parser',
      'type': 'static_library',
      'include_dirs': [ 'httpxx/libs/http-parser/' ],
      'direct_dependent_settings': {
        'include_dirs': [ 'httpxx/libs/http-parser/' ],
      },
      'defines': [ 'HTTP_PARSER_STRICT=0' ],
      'sources': [ 'httpxx/libs/http-parser/http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              # Compile as C++. http_parser.c is actually C99, but C++ is
              # close enough in this case.
              'CompileAs': 2,
            },
          },
        }]
      ],
    },{
      'target_name': 'test',
      'type': 'executable',
      'dependencies': [ 'http_parser' ],
      'sources': [ 'httpxx/libs/http-parser/test.c' ]
    }
  ]
}

