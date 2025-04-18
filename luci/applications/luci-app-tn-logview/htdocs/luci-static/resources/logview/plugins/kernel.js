'use strict';

return L.Class.extend({
	title: _('Kernel Logs'),
	description: 'Kernel logs',
	order: 3,
	acl: 'luci-app-tn-logview-kernel',
	json_data: {
		add_to_downloads: true,
		action: {
			command: '/usr/libexec/luci-logview/logview-dmesg',
			command_args: ['json' ]
		}
	},
	downloads: {
		'raw': {
			display: 'RAW',
			mime_type: 'text/plain',
			extension: 'txt',
			action: {
				command: '/usr/libexec/luci-logview/logview-dmesg',
				command_args: [ 'plain' ]
			}
		},
		'csv': {
			display: 'CSV',
			mime_type: 'text/csv',
			extension: 'csv',
			action: {
				command: '/usr/libexec/luci-logview/logview-dmesg',
				command_args: [ 'csv' ]
			}
		},
		'clearlog': {
			display: 'clearlog',
			mime_type: 'text/csv',
			extension: 'csv',
			action: {
				command: '/usr/libexec/luci-logview/logview-dmesg',
				command_args: [ 'clearlog' ]
			}
		}
	},
	columns: [
		{ name: 'ktime', display: _('Time') },
		{ name: 'priority', display: _('Priority') },
		{ name: 'message', display: _('Message') }
	]
});
