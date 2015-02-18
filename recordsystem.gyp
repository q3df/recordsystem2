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
			'Q3DF_VERSION="<!@(git describe --tags --long --match \"v*\")"',
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
			'tools/boost/include/',
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
			'client/Q3Env.cc',
			'client/Q3Env.h',
			'tools/jsoncpp/jsoncpp.cpp',
			'tools/jsoncpp/json/json.h',
			'tools/jsoncpp/json/json-forwards.h',
		],
		'conditions': [
			['OS=="linux"', {
				'defines': [
					'LINUX',
					'Q3DF_BUILD=\"<!@(date) (posix-x86)\"',
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
		        ['OS == "mac"', {
				'xcode_settings': {
        			'OTHER_CFLAGS': [
        				'-stdlib=libc++', '-std=c++11'
        			],
			    },
				'defines': [
					'LINUX',
				],
				'include_dirs': [
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
    						'$(SDKROOT)/System/Library/Frameworks/CoreServices.framework',
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
			'tools/pthreads-win32/pthread.gyp:pthreads',
			'tools/q3df_api/q3df_api.gyp:q3df_api',
			'tools/protobuf/protobuf.gyp:protobuf_full_do_not_use',
			'tools/sqlite/sqlite.gyp:sqlite',
			'tools/httpxx.gyp:httpxx'
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protobuf/src/',
			'tools/q3df_api/',
			'tools/mysql/include/',
			'tools/httpxx/code/',
			'tools/httpxx/libs/http-parser/',
			'tools/boost/include/'
		],
		'actions': [],
		'sources': [
			'server/Commands/TopCommand.cc',
			'server/Commands/AdminCommand.cc',
			'server/RecordsystemDaemon.cc',
			'server/RecordsystemDaemon.h',
			'server/Console.cc',
			'server/Console.h',
			'server/Q3dfEnv.cc',
			'server/Q3dfEnv.h',
			'server/Q3dfApiImpl.cc',
			'server/Q3dfApiImpl.h',
			'server/ClientList.cc',
			'server/ClientList.h',
			'server/MysqlPool.cc',
			'server/MysqlPool.h',
			'server/Commands.cc',
			'server/Commands.h',
			'server/RconClient.cc',
			'server/RconClient.h',
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
						'tools/precompiled/linux_x86/libmysqlcppconn-static.a',
						'tools/precompiled/linux_x86/libmysqlclient.a',
						'tools/precompiled/linux_x86/libboost_program_options.a',
						'-lm',
						'-ldl',
						'-lpthread',
						'-lrt',
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
			['OS == "mac"', {
				'xcode_settings': {
	        			'OTHER_CFLAGS': [
        					'-stdlib=libc++', '-std=c++11'
        				],
				},
				'defines': [
					'LINUX', 'MAC'
				],
				'sources': [
					'server/ConsoleTty.cc',
					'server/ConsoleTty.h',
				],
				'link_settings': {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
    					'$(SDKROOT)/System/Library/Frameworks/CoreServices.framework',
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
				'include_dirs': [
					'tools/inttypes_win/'
				],
				'msvs_settings': {
					'VCCLCompilerTool': {
						'DisableSpecificWarnings': ['4251'],
					},
					'VCLinkerTool': {
						'AdditionalLibraryDirectories': [
							'tools/precompiled/win_x86/'
						],
						'AdditionalDependencies': [
							'Advapi32.lib',
							'kernel32.lib',
							'user32.lib',
							'gdi32.lib',
							'winspool.lib',
							'shell32.lib',
							'ole32.lib',
							'oleaut32.lib',
							'uuid.lib',
							'comdlg32.lib',
							'advapi32.lib',
							'ws2_32.lib',
							'Secur32.lib',
							'libmysql.lib',
							'mysqlcppconn.lib',
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
