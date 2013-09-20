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
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protorpc/src/'
		],
		'actions': [{
			'action_name': 'protoc echoService compiling',
			'inputs': [
				'protoc/echoservice.proto'
			],
			'outputs': [
				'protoc/echoservice.pb.cc',
				'protoc/echoservice.pb.h',
			],
			'action': [
				'./compileprotoc.sh',
				'protoc',
				'echoservice.proto'
			],
		}],
		'sources': [
			'protoc/echoservice.pb.cc',
			'protoc/echoservice.pb.h',
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
		],
		'conditions': [
			['OS=="linux"', {
				'defines': [
					'LINUX',
				],
				'include_dirs': [
				],
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
		],
		'defines': [
		],
		'include_dirs': [
			'tools/protorpc/src/'
		],
		'actions': [{
			'action_name': 'protoc echoService compiling',
			'inputs': [
				'protoc/echoservice.proto'
			],
			'outputs': [
				'protoc/echoservice.pb.cc',
				'protoc/echoservice.pb.h',
			],
			'action': [
				'./compileprotoc.sh',
				'protoc',
				'echoservice.proto'
			],
		}],
		'sources': [
			'protoc/echoservice.pb.cc',
			'protoc/echoservice.pb.h',
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
