{
	'targets': [{
		'target_name': 'pthreads',
		'msvs_guid': '39B03BBB-062B-431E-B4A6-49B7406B102D',
		'conditions': [
			['OS!="win"', {
				'type': 'none',
			},{
				'type': 'shared_library',
				'sources': [
					'pthread.c',
					'version.rc',
				],
				'defines': [
					'PTW32_BUILD',
					'PTW32_RC_MSC',
					# 'PTW32_STATIC_LIB',
				],
				'msvs_settings': {
					'VCLinkerTool': {
						'ProgramDatabaseFile': '$(OutDir)\\pthreads_dll.pdb',
					},
				},
				'link_settings': {
					'libraries': [
						'-lws2_32.lib',
					]
				},
				'include_dirs': [
					'./'
				],
				'direct_dependent_settings': {
					'include_dirs': [
						'./'
					],
					'defines': [
						# 'PTW32_STATIC_LIB',
					],
				},
			}],
		],
	}]
}
