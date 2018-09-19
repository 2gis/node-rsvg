{
	"variables": {
		"GTK_Root%": "c:\\gtk",
		"conditions": [
			[ "OS == 'mac'", {
				"pkg_env": "PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig"
			}, {
				"pkg_env": ""
			}]
		]
	},
	"targets": [
		{
			"target_name": "rsvg",
			"sources": [
				"src/Rsvg.cc",
				"src/Enums.cc",
				"src/Autocrop.cc"
			],
			"include_dirs": [
				"<!(node -e \"require('nan')\")" 
			],
			"variables": {
				"packages": "librsvg-2.0 cairo-png cairo-pdf cairo-ps cairo-svg",
				"conditions": [
					[ "OS!='win'", {
						"libraries": "<!(<(pkg_env) pkg-config --libs-only-l <(packages))",
						"ldflags": "<!(<(pkg_env) pkg-config --libs-only-L --libs-only-other <(packages))",
						"cflags": "<!(<(pkg_env) pkg-config --cflags <(packages))"
					}, { # else OS!='win'
						"include_dirs": "<!(<(python) tools/include_dirs.py <(GTK_Root) <(packages))"
					} ]
				]
			},
			"conditions": [
				[ "OS!='mac' and OS!='win'", {
					"cflags": [
						"<@(cflags)",
						"-fpermissive",
						"-std=c++0x"
					],
					"ldflags": [
						"<@(ldflags)"
					],
					"libraries": [
						"<@(libraries)"
					],
				} ],
				[ "OS=='mac'", {
					"xcode_settings": {
						"OTHER_CFLAGS": [
							"<@(cflags)"
						],
						"OTHER_LDFLAGS": [
							"<@(ldflags)"
						]
					},
					"libraries": [
						"<@(libraries)"
					],
				} ],
				[ "OS=='win'", {
					"sources+": [
						"src/win32-math.cc"
					],
					"include_dirs": [
						"<@(include_dirs)"
					],
					"libraries": [
						'librsvg-2.dll.a',
						'glib-2.0.lib',
						'gobject-2.0.lib',
						'cairo.lib'
					],
					"msvs_settings": {
						'VCCLCompilerTool': {
							'AdditionalOptions': [
								"/EHsc"
							]
						}
					},
					"msbuild_settings": {
						"Link": {
							"AdditionalLibraryDirectories": [
								"<(GTK_Root)\\lib"
							],
							"ImageHasSafeExceptionHandlers": "false"
						}
					}
				} ]
			]
		}
	]
}
