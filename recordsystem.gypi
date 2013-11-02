{
	'target_defaults': {
		'default_configuration': 'Debug',
		'configurations': {
			# ------------------------------------------------------------------
			# DEUB CONFIG
			# ------------------------------------------------------------------
			'Debug': {
				'defines': [
					'DEBUG',
				],
				'conditions': [
					['OS=="linux"', {
					}],
					['OS=="win"', {
						'msvs_settings': {
							'VCCLCompilerTool': {
								'Optimization': '0',
								'DebugInformationFormat': '3',
							},
							'VCLinkerTool': {
								'AdditionalOptions': [
									'/Debug',
								],
								'ProgramDatabaseFile': '$(TargetPath).pdb'
							},
						},
					}],
				]
			},

			# ------------------------------------------------------------------
			# RELEASE CONFIG
			# ------------------------------------------------------------------
			'Release': {
				'defines': [
					'NDEBUG',
				],
				'conditions': [
					['OS=="linux"', {
						'cflags!': [
							'-O2',
							'-Os',
						],
						'cflags': [
							'-O3',
						],
					}],
					['OS=="win"', {
						'msvs_settings': {
							'VCCLCompilerTool': {
								'Optimization': '2',
								'InlineFunctionExpansion': '2',
								'EnableIntrinsicFunctions': 'true',
								'FavorSizeOrSpeed': '0',
								'StringPooling': 'true',
							},
							'VCLinkerTool': {
								'OptimizeReferences': '2',
								'EnableCOMDATFolding': '2',
							},
						},
					}],
				],
			},
		},
	},
	'conditions': [
		# ------------------------------------------------------------------
		# DEFAULT CONFIG FOR RELEASE/DEBUG
		# ------------------------------------------------------------------
		['OS=="win"', {
			'target_defaults': {
				'defines': [ 'WIN32', '_CRT_SECURE_NO_DEPRECATE', '_CRT_NONSTDC_NO_DEPRECATE' ],
				'msvs_disabled_warnings': [4996, 4541, 4355, 4800],
				'msvs_configuration_attributes': {
					'OutputDirectory': '<(DEPTH)\\build\\$(ConfigurationName)',
					'IntermediateDirectory': '$(OutDir)\\obj\\$(ProjectName)',
					'CharacterSet': '1',
				},
				'msvs_settings': {
					'VCCLCompilerTool': {
						'RuntimeLibrary': '2',
						'MinimalRebuild': 'false',
						'BufferSecurityCheck': 'true',
						'EnableFunctionLevelLinking': 'true',
						'RuntimeTypeInfo': 'true',
						'WarningLevel': '3',
						'WarnAsError': 'true',
						'Detect64BitPortabilityProblems': 'false',
						'AdditionalOptions': ['/MP'],
						'ExceptionHandling': '1',  # /EHsc
					},
					'VCLibrarianTool': {
						'AdditionalOptions': [
						],
					},
					'VCLinkerTool': {
						'AdditionalDependencies': [
						],
						'GenerateDebugInformation': 'true',
						'MapFileName': '$(OutDir)\\$(TargetName).map',
						'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
						'FixedBaseAddress': '1',
						# LinkIncremental values:
						#   0 == default
						#   1 == /INCREMENTAL:NO
						#   2 == /INCREMENTAL
						'LinkIncremental': '1',
						# SubSystem values:
						#   0 == not set
						#   1 == /SUBSYSTEM:CONSOLE
						#   2 == /SUBSYSTEM:WINDOWS
						'SubSystem': '1',
					},
				},
			},
		}],
		['OS=="linux"', {
			'target_defaults': {
				'cflags': [
					'-Wall', '-Werror', '-W', '-Wno-unused-parameter',
					'-pthread', '-fno-exceptions', '-pedantic',
					'-std=c++0x', '-fPIC', '-fexceptions', '-fpermissive',
					'-m32', '-L/usr/lib32', '-Wno-strict-aliasing'
				],
				'cflags_cc': [
				],
				'defines': [ 'LINUX' ],
				'ldflags': [ '-std=c++0x', '-m32', '-L/usr/lib32' ],
			}
		}],
	],
}
