{
	'targets': [{
		'target_name': 'qagamex86',
		'type': 'shared_library',
		'msvs_guid': '905C1895-E657-4584-B1C9-C8642BDA8523',
		'dependencies': [
		],
		'defines': [
		],
		'include_dirs': [
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
				'include_dirs': [
				],
			}],
		],
	}],
}
