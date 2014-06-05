# This file is used with the GYP meta build system.
# http://code.google.com/p/gyp/
{
	'targets': [{
		'target_name': 'qagamex86',
		'type': 'shared_library',
		'msvs_guid': '905C1895-E657-4584-B1C9-C8642BDA8523',
		'dependencies': [
			'tools/sqlite/sqlite.gyp:sqlite',
			'tools/pthreads-win32/pthread.gyp:pthreads',
			'tools/q3df_api/q3df_api.gyp:q3df_api',
			'tools/protobuf/protobuf.gyp:protobuf_full_do_not_use',
			'tools/httpxx.gyp:httpxx'
		],
		'defines': [
			'JSON_IS_AMALGAMATION',
			'Q3DF_VERSION="<!@(echo 1.4)"',
		],
		'include_dirs': [
			'tools/protobuf/src/',
			'tools/utf8/',
			'tools/q3df_api/',
			'tools/protobuf/third_party/snappy/',
			'tools/protobuf/third_party/tinyxml/include/',
			'tools/httpxx/code/',
			'tools/httpxx/libs/http-parser/',
			'tools/jsoncpp/',
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
			'client/SqliteDatabase.cc',
			'client/SqliteDatabase.h',
			'tools/jsoncpp/jsoncpp.cpp',
			'tools/jsoncpp/json/json.h',
			'tools/jsoncpp/json/json-forwards.h',
		],
		'conditions': [
			['OS=="linux"', {
				'defines': [
					'LINUX',
					#'Q3DF_BUILD="<!@(date) (posix-x86)")',
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
					'Q3DF_BUILD="<!@(echo %date% %time%) (win-x86)"',
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
			'tools/sqlite/sqlite.gyp:sqlite',
			'modules/modules.gyp:modules',
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protobuf/src/',
			'tools/q3df_api/',
			'tools/mysql/',
		],
		'actions': [],
		'sources': [
			'server/RecordsystemDaemon.cc',
			'server/RecordsystemDaemon.h',
			'server/Console.cc',
			'server/Console.h',
			'server/Q3dfEnv.cc',
			'server/Q3dfEnv.h',
			'server/Q3dfApiImpl.cc',
			'server/Q3dfApiImpl.h',
			'server/CompatibilityManager.cc',
			'server/CompatibilityManager.h',
			'client/StringTokenizer.cc',
			'client/StringTokenizer.h',
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
						'-L/usr/lib/i386-linux-gnu <!@(mysql_config --libs)'
					],
					'library_dirs': [
					]
				},
				'sources': [
					'server/ConsoleTty.cc',
					'server/ConsoleTty.h',
				],
				'cflags': [
					'-std=c++0x', '-fPIC', '-fexceptions', '-fpermissive', '<!@(mysql_config --cflags)'
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
							'tools/mysql/libmysql.lib'
						],
						'AdditionalOptions': [
						]
					},
				},
				'sources': [
					'server/ConsoleWin32.cc',
					'server/ConsoleWin32.h',
				],
			}],
		],
	}],
}
