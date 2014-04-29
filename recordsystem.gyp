{
#	'make_global_settings': [
#		['CXX','/usr/bin/g++'],
# 		['LINK','/usr/bin/g++'],
#	],
	'targets': [{
		'target_name': 'qagamex86',
		'type': 'shared_library',
		'msvs_guid': '905C1895-E657-4584-B1C9-C8642BDA8523',
		'dependencies': [
			'tools/libxml/libxml.gyp:libxml',
			'tools/pthreads-win32/pthread.gyp:pthreads',
			'tools/q3df_api/q3df_api.gyp:q3df_api',
			'tools/protobuf/protobuf.gyp:protobuf_full_do_not_use'
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protobuf/src/',
			'tools/utf8/',
			'tools/q3df_api/'
		],
		'sources': [
			'client/Q3Vm.cc',
			'client/Q3Vm.h',
			'client/Q3SysCall.cc',
			'client/Q3SysCall.h',
			'client/Q3Event.h',
			'client/Q3User.cc',
			'client/Q3User.h',
			'client/Quake3.h',
			'client/Logger.cc',
			'client/Logger.h',
			'client/PluginStore.cc',
			'client/PluginStore.h',
			'client/PluginProxy.h',
			'client/PluginBase.h',
			'client/Recordsystem.cc',
			'client/Recordsystem.h',
			'client/EntryPoint.cc',
			'client/EntryPoint.h',
			'client/ApiAsyncExecuter.cc',
			'client/ApiAsyncExecuter.h',
			'client/VmCvarItem.cc',
			'client/VmCvarItem.h',
			'client/Utils.cc',
			'client/Utils.cc',
			'client/StringTokenizer.cc',
			'client/StringTokenizer.h',
			'client/Utils.h',
			'client/Plugins/UserManagementPlugin.cc',
			'client/Plugins/ServerDemosPlugin.cc',
			'client/Plugins/ServerRecordsPlugin.cc',
			'client/Plugins/UserCommandsPlugin.cc',
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
						'-pthread'
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
					'VCLinkerTool': {
						'AdditionalDependencies': [
							'Advapi32.lib',
						],
					},
				},
			}],
		],
	},{
		'target_name': 'recordsystemd',
		'type': 'executable',
		'msvs_guid': 'F3706CA9-3A31-4B33-8CF6-73EAAC40BF1E',
		'dependencies': [
			'qagamex86',
			'tools/pthreads-win32/pthread.gyp:pthreads',
			'tools/q3df_api/q3df_api.gyp:q3df_api',
			'tools/protobuf/protobuf.gyp:protobuf_full_do_not_use',
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protobuf/src/',
			'tools/q3df_api/'
		],
		'actions': [],
		'sources': [
			'server/RecordsystemDaemon.cc',
			'server/RecordsystemDaemon.h',
			'server/Console.cc',
			'server/Console.h',
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
						'-pthread'
					],
					'library_dirs': [
					]
				},
				'sources': [
					'server/ConsoleTty.cc',
					'server/ConsoleTty.h',
				],
				'cflags': [
					'-std=c++0x', '-fPIC', '-fexceptions', '-fpermissive'
				]
			}],
			['OS=="win"', {
				'defines': [
					'WIN32',
				],
				'sources': [
					'server/ConsoleWin32.cc',
					'server/ConsoleWin32.h',
				],
			}],
		],
	}],
}
