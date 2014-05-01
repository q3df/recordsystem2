{
	'targets': [{
		'target_name': 'q3df_api',
		'type': 'static_library',
		'msvs_guid': 'EEE489A6-F54C-4A20-945F-326EAB9654E0',
		'dependencies': [
			'../protobuf/protobuf.gyp:protoc',
		],
		'defines': [
		],
		'include_dirs': [
			'../protobuf/src/'
		],
		'actions': [{
			'action_name': 'protoc q3df_api compiling',
			'inputs': [
				'q3df_api.proto'
			],
			'outputs': [
				'q3df_api.pb.cc',
				'q3df_api.pb.h',
			],
			'action': [
				'../../compileprotoc.sh',
				'./',
				'q3df_api.proto'
			],
		}],
		'sources': [
			'q3df_api.pb.cc',
			'q3df_api.pb.h',
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
				},
			}],
		],
	}],
}
