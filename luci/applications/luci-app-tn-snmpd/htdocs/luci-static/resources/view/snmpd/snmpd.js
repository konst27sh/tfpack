'use strict';
'require rpc';
'require form';
'require uci';
'require ui';
'require fs';

return L.view.extend({

	__init__: function() {
		this.super('__init__', arguments);

		this.ro_community     = null;
		this.ro_community_src = null;
		this.rw_community     = null;
		this.rw_community_src = null;
		this.ip_protocol      = null;
		this.snmp_version     = null;
	},

	/** @private */
	populateGlobalSettings: function(tab, s, data) {
		var o;

		// Service enable/disable
		o = s.taboption(tab, form.Flag, 'enabled',_('Enable SNMP'));
		o.forcewrite = true;
		o.rmempty = false;
		o.optional = false;
		o.default = '0';
		o.cfgvalue = function(section_id) {
			return uci.get('snmpd', section_id, 'enabled') || '0';
		};

		// Port
		o = s.taboption(tab, form.Value, 'port', _('Port'));
		o.rmempty = false;
		o.default = '161';
		o.datatype = 'port';
		o.forcewrite = true;
	},

	/** @private */
	populateV1Settings: function(tab, s, data) {
		// -------------------------------------------------------------------
		//
		// SNMPv1 options
		//
		// -------------------------------------------------------------------
		var o, o_src;

		//enable version
		o = s.taboption(tab, form.Flag, 'enabled_v1',_('Enable SNMPv1'));
		o.optional = false;
		o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v1', 'version_state');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v1', 'version_state',value);
		};

		// SNMPv1/SNMPv2c Read only community
		o = s.taboption(tab, form.Value, 'read_community_v1',
			_('Read community'));
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v1', 'read_community');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v1', 'read_community',value);
		};


		o = s.taboption(tab, form.Value, 'read_access',
			_('Read community source'),
			_('Trusted source for SNMP read community access (hostname, IP/MASK, IP/BITS or IPv6 equivalents)'));
		o.value('default', _('any (default)'));
		o.value('localhost', 'localhost');
		o.default = 'default';
		o.rmempty = false;
		o.datatype = 'or(host(0),ipmask)';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v1', 'read_access');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v1', 'read_access',value);
		};



		//Read/write community
		o = s.taboption(tab, form.Value, 'write_community_v1',
			_('Write community'));
		//this.rw_community.default = 'private';
		o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v1', 'write_community');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v1', 'write_community',value);
		};

		o = s.taboption(tab, form.Value, 'write_access',
			_('Write community source'),
			_('Trusted source for SNMP write community access (hostname, IP/MASK, IP/BITS or IPv6 equivalents)'));
		o.value('default', _('any (default)'));
		o.value('localhost', 'localhost');
		o.default = 'localhost';
		o.rmempty = false;
		o.datatype = 'or(host(0),ipmask)';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v1', 'write_access');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v1', 'write_access',value);
		};

	},

	/** @private */
	populateV2Settings: function(tab, s, data) {
		// -------------------------------------------------------------------
		//
		// SNMPv2c options
		//
		// -------------------------------------------------------------------
		var o, o_src;

		//enable version
		o = s.taboption(tab, form.Flag, 'enabled_v2',_('Enable SNMPv2'));
		o.optional = false;
		o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v2c', 'version_state');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v2c', 'version_state',value);
		};

		// SNMPv1/SNMPv2c Read only community
		o = s.taboption(tab, form.Value, 'read_community_v2',
			_('Read community'));
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v2c', 'read_community');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v2c', 'read_community',value);
		};


		o = s.taboption(tab, form.Value, 'read_access',
			_('Read community source'),
			_('Trusted source for SNMP read community access (hostname, IP/MASK, IP/BITS or IPv6 equivalents)'));
		o.value('default', _('any (default)'));
		o.value('localhost', 'localhost');
		o.default = 'default';
		o.rmempty = false;
		o.datatype = 'or(host(0),ipmask)';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v2c', 'read_access');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v2c', 'read_access',value);
		};



		//Read/write community
		o = s.taboption(tab, form.Value, 'write_community_v2',	_('Write community'));
		o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v2c', 'write_community');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v2c', 'write_community',value);
		};

		o = s.taboption(tab, form.Value, 'write_access',
			_('Write community source'),
			_('Trusted source for SNMP write community access (hostname, IP/MASK, IP/BITS or IPv6 equivalents)'));
		o.value('default', _('any (default)'));
		o.value('localhost', 'localhost');
		o.default = 'localhost';
		o.rmempty = false;
		o.datatype = 'or(host(0),ipmask)';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v2c', 'write_access');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v2c', 'write_access',value);
		};
	},

	/** @private */
	populateV3Settings: function(tab, s, data) {
		// -------------------------------------------------------------------
		//
		// SNMPv3 options
		//
		// -------------------------------------------------------------------
		var o;

		o = s.taboption(tab, form.Flag, 'enabled_v3',_('Enable SNMPv3'));
		o.default = '0';
		o.rmempty = false;
		o.optional = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'version_state');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'version_state',value);
		};

		// SNMPv3 user name
		o = s.taboption(tab, form.Value, 'snmp_v3_username',
			_('SNMPv3 username'),
			_('Set username to access SNMP'));
		o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'username');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'username',value);
		};

		// SNMPv3 auth type
		o = s.taboption(tab, form.ListValue, 'snmp_v3_auth_type',
			_('SNMPv3 authentication type'));
		o.value('none', _('none'));
		o.value('SHA', _('SHA'));
		o.value('MD5', _('MD5'));
		o.rmempty = false;
		o.default = 'SHA';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'auth_type');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'auth_type',value);
		};

		// SNMPv3 auth pass
		o = s.taboption(tab, form.Value, 'snmp_v3_auth_pass',
			_('SNMPv3 authentication passphrase'));
		o.password = true;
		//o.rmempty = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'auth_pass');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'auth_pass',value);
		};


		// SNMPv3 privacy/encryption type
		o = s.taboption(tab, form.ListValue, 'snmp_v3_privacy_type',
			_('SNMPv3 encryption type'));
		o.value('none', _('none'));
		o.value('AES', _('AES'));
		o.value('DES', _('DES'));
		//o.rmempty = false;
		o.default = 'AES';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'privacy_type');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'privacy_type',value);
		};

		// SNMPv3 privacy/encryption pass
		o = s.taboption(tab, form.Value, 'snmp_v3_privacy_pass',
			_('SNMPv3 encryption passphrase'));
		o.password = true;
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'privacy_pass');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'privacy_pass',value);
		};

		// SNMPv3 write allow
		o = s.taboption(tab, form.Flag, 'snmp_v3_allow_write',
			_('Allow write'));
		//o.rmempty = false;
		o.default = '0';
		o.load = function(section_id) {
			return uci.get('snmpd', 'snmp_v3', 'allow_write');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'snmp_v3', 'allow_write',value);
		};
	},

	/** @private */
	populateTrapsSettings: function(tab, s, data) {
		// -------------------------------------------------------------------
		//
		// Trap settings
		//
		// -------------------------------------------------------------------
		var trap_enable;
		var trap_snmp_version;
		var trap_host;
		var trap_port;
		var trap_community;

		// Trap enable
		trap_enable = s.taboption(tab, form.Flag, 'trap_enabled',
			_('Enable SNMP traps'),
			_('Enable SNMP trap functionality'));
		trap_enable.default = '0';
		trap_enable.rmempty = false;
		trap_enable.forcewrite = true;
		trap_enable.load = function(section_id) {
			return uci.get('snmpd', 'trap', 'trap_enabled');
		};
		trap_enable.write = function(section_id, value) {
			uci.set('snmpd', 'trap', 'trap_enabled',value);
		};

		// Trap SNMP version
		trap_snmp_version = s.taboption(tab, form.ListValue, 'trap_snmp_version',
			_('SNMP traps version'),
			_('SNMP version used for sending traps'));
		trap_snmp_version.value('v1', 'SNMPv1');
		trap_snmp_version.value('v2c', 'SNMPv2c');
		trap_snmp_version.default = 'v2c';
		trap_snmp_version.load = function(section_id) {
			return uci.get('snmpd', 'trap', 'trap_snmp_version');
		};
		trap_snmp_version.write = function(section_id, value) {
			uci.set('snmpd', 'trap', 'trap_snmp_version',value);
		};

		// Trap host
		trap_host = s.taboption(tab, form.Value, 'trap_host',
			_('Host/IP'),
			_('Host to transfer SNMP trap traffic to (hostname or IP address)'));
		trap_host.datatype = 'host(0)';
		trap_host.default = 'localhost';
		trap_host.rmempty = false;
		trap_host.load = function(section_id) {
			return uci.get('snmpd', 'trap', 'trap_host');
		};
		trap_host.write = function(section_id, value) {
			uci.set('snmpd', 'trap', 'trap_host',value);
		};

		// Trap port
		trap_port = s.taboption(tab, form.Value, 'trap_port',
			_('Port'),
			_('Port for trap\'s host'));
		trap_port.default = '162';
		trap_port.datatype = 'port';
		trap_port.rmempty = false;
		trap_port.load = function(section_id) {
			return uci.get('snmpd', 'trap', 'trap_port');
		};
		trap_port.write = function(section_id, value) {
			uci.set('snmpd', 'trap', 'trap_port',value);
		};

		// Trap community
		trap_community = s.taboption(tab, form.Value, 'trap_community',
			_('Community'),
			_('The SNMP community for traps'));
		trap_community.value('public');
		trap_community.value('private');
		trap_community.default = 'public';
		trap_community.rmempty = false;
		trap_community.load = function(section_id) {
			return uci.get('snmpd', 'trap', 'trap_community');
		};
		trap_community.write = function(section_id, value) {
			uci.set('snmpd', 'trap', 'trap_community',value);
		};
	},



	/** @private */
	populateLogSettings: function(tab, s, data) {
		// -------------------------------------------------------------------
		//
		// Logging
		//
		// -------------------------------------------------------------------
		var o;
		// Syslog
		o = s.taboption(tab, form.Flag, 'log_syslog',
			_('Enable logging to syslog'));
		o.default = '0';
		o.rmempty = false;
		o.optional = false;
		o.load = function(section_id) {
			return uci.get('snmpd', 'log', 'log_syslog');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'log', 'log_syslog',value);
		};

		o = s.taboption(tab, form.ListValue, 'log_syslog_facility',
			_('Syslog facility'));
		o.default = 'i';
		o.value('d', 'LOG_DAEMON');
		o.value('u', 'LOG_USER');
		o.value('0', 'LOG_LOCAL0');
		o.value('1', 'LOG_LOCAL1');
		o.value('2', 'LOG_LOCAL2');
		o.value('3', 'LOG_LOCAL3');
		o.value('4', 'LOG_LOCAL4');
		o.value('5', 'LOG_LOCAL5');
		o.value('6', 'LOG_LOCAL6');
		o.value('7', 'LOG_LOCAL7');
		o.depends('log_syslog', '1');
		o.load = function(section_id) {
			return uci.get('snmpd', 'log', 'log_syslog_facility');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'log', 'log_syslog_facility',value);
		};

		o = s.taboption(tab, form.ListValue, 'log_syslog_priority',
			_('Priority for syslog logging'),
			_('Will log messages of selected priority and above.'));
		o.default = 'i';
		o.value('!', 'LOG_EMERG');
		o.value('a', 'LOG_ALERT');
		o.value('c', 'LOG_CRIT');
		o.value('e', 'LOG_ERR');
		o.value('w', 'LOG_WARNING');
		o.value('n', 'LOG_NOTICE');
		o.value('i', 'LOG_INFO');
		o.value('d', 'LOG_DEBUG');
		o.depends('log_syslog', '1');
		o.load = function(section_id) {
			return uci.get('snmpd', 'log', 'log_syslog_priority');
		};
		o.write = function(section_id, value) {
			uci.set('snmpd', 'log', 'log_syslog_priority',value);
		};
	},

	render: function(data) {
		var m, s, o;

		m = new form.Map('snmpd',
			_('SNMP Settings'),
			_('On this page you may configure SNMP agent settings.'));

		s = m.section(form.TypedSection, 'general');
		s.anonymous = true;
		s.addremove = false;

		s.tab('global', _('Global'));
		s.tab('snmp_v1',  _('SNMPv1'));
		s.tab('v2c',  _('SNMPv2c'));
		s.tab('v3',     _('SNMPv3'));
		s.tab('traps',  _('Traps', 'SNMP'));
		s.tab('log',    _('Logging'));

		this.populateGlobalSettings ('global', s, data);
		this.populateV1Settings  ('snmp_v1',  s, data);
		this.populateV2Settings  ('v2c',  s, data);
		this.populateV3Settings     ('v3',     s, data);
		this.populateTrapsSettings  ('traps',  s, data);
		this.populateLogSettings    ('log',    s, data);

		return m.render();
	}
});
