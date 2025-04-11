'use strict';

return L.Class.extend({
	title: _('Switch Logs'),
	description: 'Switch logs, stored in flash memory',
	order: 2,
	acl: 'luci-app-tn-logview-switch',
	json_data: {
		add_to_downloads: true,
		action: {
			command: '/usr/libexec/luci-logview/logview-switch',
			command_args: [ 'json' ]
		}
	},
	downloads: {
		'raw': {
			display: 'RAW',
			mime_type: 'text/plain', extension: 'txt',
			action: {
				command: '/usr/libexec/luci-logview/logview-switch',
				command_args: [ 'plain' ]
			}
		},
		'csv': {
			display: 'CSV',
			mime_type: 'text/csv',
			extension: 'csv',
			action: {
				command: '/usr/libexec/luci-logview/logview-switch',
				command_args: [ 'csv' ]
			}
		},
		'clearlog': {
			display: 'clearlog',
			mime_type: 'text/csv',
			extension: 'csv',
			action: {
				command: '/usr/libexec/luci-logview/logview-switch',
				command_args: [ 'clearlog' ]
			}
		}
	},
	columns: [
		{ name: 'timestamp', display: _('Timestamp') },
		{ name: 'tag', display: _('Tag') },
		{ name: 'priority', display: _('Priority') },
		{ name: 'facility', display: _('Facility') },
		{ name: 'message', display: _('Message') }
	]
});
