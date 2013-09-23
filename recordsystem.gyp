{
	'make_global_settings': [
		['CXX','/usr/bin/g++'],
 		['LINK','/usr/bin/g++'],
	],
	'targets': [{
		'target_name': 'qagamex86',
		'type': 'shared_library',
		'msvs_guid': '905C1895-E657-4584-B1C9-C8642BDA8523',
		'dependencies': [
			'tools/pthreads-win32/pthread.gyp:pthreads',
			'tools/libQ3dfApi/libQ3dfApi.gyp:Q3dfApi'
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protorpc/src/',
			'tools/utf8/',
			'tools/libQ3dfApi/'
		],
		'sources': [
			'client/Q3Vm.cc',
			'client/Q3Vm.h',
			'client/Q3SysCall.cc',
			'client/Q3SysCall.h',
			'client/Q3SysCallHook.cc',
			'client/Q3SysCallHook.h',
			'client/Quake3.h',
			'client/Logger.cc',
			'client/Logger.h',
			'client/Recordsystem.cc',
			'client/Recordsystem.h',
			'client/EntryPoint.cc',
			'client/EntryPoint.h',
			'client/ApiAsyncExecuter.cc',
			'client/ApiAsyncExecuter.h',
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
						'tools/protorpc/lib/libprotobuf-posix32.a',
						'-pthread'
					],
					'library_dirs': [
					]
				},
			}],
			['OS=="win"', {
				'defines': [
					'WIN32',
				],
				'msvs_settings': {
					'VCLinkerTool': {
						'AdditionalDependencies': [
							'tools/protorpc/lib/protobuf-win32.lib'
						],
					},
				},
				'include_dirs': [
				],
			}],
		],
	},{
		'target_name': 'recordsystemd',
		'type': 'executable',
		'msvs_guid': 'F3706CA9-3A31-4B33-8CF6-73EAAC40BF1E',
		'dependencies': [
			'qagamex86',
			'tools/libQ3dfApi/libQ3dfApi.gyp:Q3dfApi'
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protorpc/src/',
			'tools/libQ3dfApi/'
		],
		'actions': [],
		'sources': [
			'server/RecordsystemDaemon.cc',
			'server/RecordsystemDaemon.h',
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
						'tools/protorpc/lib/libprotobuf-posix32.a',
						'-pthread'
					],
					'library_dirs': [
					]
				},
			}],
			['OS=="win"', {
				'defines': [
					'WIN32',
				],
				'msvs_settings': {
					'VCLinkerTool': {
						'AdditionalDependencies': [
							'tools/protorpc/lib/protobuf-win32.lib'
						],
					},
				},
				'include_dirs': [
				],
			}],
		],
	}],
}
