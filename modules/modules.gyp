{
	'targets': [{
		'target_name': 'modules',
		'type': 'shared_library',
		'msvs_guid': '51CF35C5-B2FC-4D76-978F-B67B7F68C917',
		'dependencies': [
		],
		'defines': [
		],
		'include_dirs': [
			'../tools/mysql'
		],
		'sources': [
			'modules.c',
			'modules.h',
			'top.c',
		],
		'conditions': [
			['OS=="linux"', {
				'defines': [
					'LINUX',
				],
				'include_dirs': [
				],
				'link_settings': {
					'libraries': [
						'-mysql'
					],
					'library_dirs': [
					]
				},
				'cflags': [
					'-std=c++0x', '-fPIC', '-fexceptions', '-fpermissive'
				]
			}],
			['OS=="win"', {
				'defines': [
					'WIN32',
				],
				'msvs_settings': {
					'VCCLCompilerTool' : {
						'CompileAs': 2
			        },
					'VCLinkerTool': {
						'AdditionalDependencies': [
							'Advapi32.lib',
							'../tools/mysql/libmysql.lib'
						],
						'AdditionalOptions': [
						]
					},
				},
				'sources': [
				],
			}],
		],
	}],
}
