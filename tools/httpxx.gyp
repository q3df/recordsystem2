{
	'targets': [{
		'target_name': 'httpxx',
		'type': 'static_library',
		'msvs_guid': '56841729-F8EA-4849-BC7B-A8C0D99E0979',
		'dependencies': [
			'http_parser.gyp:http_parser',
		],
		'defines': [
		],
		'include_dirs': [
			'httpxx/libs/http-parser/'
		],
		'sources': [
			'httpxx/code/BufferedMessage.hpp',
			'httpxx/code/Error.hpp',
			'httpxx/code/Flags.hpp',
			'httpxx/code/icompare.hpp',
			'httpxx/code/Message.hpp',
			'httpxx/code/Method.hpp',
			'httpxx/code/Request.hpp',
			'httpxx/code/Response.hpp',
			'httpxx/code/Url.hpp',
			'httpxx/code/Error.cpp',
			'httpxx/code/Flags.cpp',
			'httpxx/code/icompare.cpp',
			'httpxx/code/Message.cpp',
			'httpxx/code/Method.cpp',
			'httpxx/code/Request.cpp',
			'httpxx/code/Response.cpp',
			'httpxx/code/Url.cpp'
		],
		'conditions': [
			['OS=="linux"', {
				'defines': [
					'LINUX',
				],
			}],
			['OS=="linux"', {
				'defines': [
					'WIN32',
				],
			}],
		],
	}],
}
