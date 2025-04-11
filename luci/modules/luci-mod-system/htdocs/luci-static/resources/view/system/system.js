'use strict';
'require view';
'require poll';
'require ui';
'require uci';
'require rpc';
'require form';
'require tools.widgets as widgets';

var callInitList, callInitAction, callTimezone,
	callGetLocaltime, callSetLocaltime, CBILocalTime;


const TZ = {
	'Etc/GMT':{ 'tzstring':'GMT0'} ,
	'Etc/GMT+1':{ 'tzstring':'<-01>1'},
	'Etc/GMT+10':{ 'tzstring': '<-10>10'},
	'Etc/GMT+11':{ 'tzstring': '<-11>11'},
	'Etc/GMT+12':{ 'tzstring':  '<-12>12'},
	'Etc/GMT+2':{ 'tzstring':  '<-02>2'},
	'Etc/GMT+3':{ 'tzstring':  '<-03>3'},
	'Etc/GMT+4':{ 'tzstring':  '<-04>4'},
	'Etc/GMT+5':{ 'tzstring':  '<-05>5'},
	'Etc/GMT+6':{ 'tzstring':  '<-06>6'},
	'Etc/GMT+7':{ 'tzstring':  '<-07>7'},
	'Etc/GMT+8':{ 'tzstring':  '<-08>8'},
	'Etc/GMT+9':{ 'tzstring':  '<-09>9'},
	'Etc/GMT-1':{ 'tzstring':  '<+01>-1'},
	'Etc/GMT-10':{ 'tzstring':  '<+10>-10'},
	'Etc/GMT-11':{ 'tzstring':  '<+11>-11'},
	'Etc/GMT-12':{ 'tzstring':  '<+12>-12'},
	'Etc/GMT-13':{ 'tzstring':  '<+13>-13'},
	'Etc/GMT-14':{ 'tzstring':  '<+14>-14'},
	'Etc/GMT-2':{ 'tzstring':  '<+02>-2'},
	'Etc/GMT-3':{ 'tzstring':  '<+03>-3'},
	'Etc/GMT-4':{ 'tzstring':  '<+04>-4'},
	'Etc/GMT-5':{ 'tzstring':  '<+05>-5'},
	'Etc/GMT-6':{ 'tzstring':  '<+06>-6'},
	'Etc/GMT-7':{ 'tzstring':  '<+07>-7'},
	'Etc/GMT-8':{ 'tzstring':  '<+08>-8'},
	'Etc/GMT-9':{ 'tzstring':  '<+09>-9'}
};

callInitList = rpc.declare({
	object: 'luci',
	method: 'getInitList',
	params: [ 'name' ],
	expect: { '': {} },
	filter: function(res) {
		for (var k in res)
			return +res[k].enabled;
		return null;
	}
});

callInitAction = rpc.declare({
	object: 'luci',
	method: 'setInitAction',
	params: [ 'name', 'action' ],
	expect: { result: false }
});

callGetLocaltime = rpc.declare({
	object: 'system',
	method: 'info',
	expect: { localtime: 0 }
});

callSetLocaltime = rpc.declare({
	object: 'luci',
	method: 'setLocaltime',
	params: [ 'localtime' ],
	expect: { result: 0 }
});

callTimezone = rpc.declare({
	object: 'luci',
	method: 'getTimezones',
	expect: { '': {} }
});

function formatTime(epoch) {
	var date = new Date(epoch * 1000);

	return '%04d-%02d-%02d %02d:%02d:%02d'.format(
		date.getUTCFullYear(),
		date.getUTCMonth() + 1,
		date.getUTCDate(),
		date.getUTCHours(),
		date.getUTCMinutes(),
		date.getUTCSeconds()
	);
}

CBILocalTime = form.DummyValue.extend({
	renderWidget: function(section_id, option_id, cfgvalue) {
		return E([], [
			E('input', {
				'id': 'localtime',
				'type': 'text',
				'readonly': true,
				'value': formatTime(cfgvalue)
			}),
			E('br'),
			E('span', { 'class': 'control-group' }, [
				E('button', {
					'class': 'cbi-button cbi-button-apply',
					'click': ui.createHandlerFn(this, function() {
						return callSetLocaltime(Math.floor(Date.now() / 1000));
					}),
					'disabled': (this.readonly != null) ? this.readonly : this.map.readonly
				}, _('Sync with browser')),
				' ',
				this.ntpd_support ? E('button', {
					'class': 'cbi-button cbi-button-apply',
					'click': ui.createHandlerFn(this, function() {
						return callInitAction('sysntpd', 'restart');
					}),
					'disabled': (this.readonly != null) ? this.readonly : this.map.readonly
				}, _('Sync with NTP-Server')) : ''
			])
		]);
	},
});

return view.extend({
	load: function() {
		return Promise.all([
			callInitList('sysntpd'),
			callTimezone(),
			callGetLocaltime(),
			uci.load('luci'),
			uci.load('system')
		]);
	},

	render: function(rpc_replies) {
		var ntpd_enabled = rpc_replies[0],
			timezones = TZ,
			localtime = rpc_replies[2],
			m, s, o;

		m = new form.Map('system',null,null	);

		m.chain('luci');

		s = m.section(form.TypedSection, 'system', _('System Properties'),_('Here you can configure the basic aspects of your device like its hostname or the timezone.'));
		s.anonymous = true;
		s.addremove = false;

		s.tab('general', _('Description'));
		s.tab('timesync', _('Time/Date'));
		s.tab('language', _('Language'));

		/*
		 * System Properties
		 */



		o = s.taboption('general', form.Value, 'hostname', _('Hostname'),_('Used to identify the device in network'));
		o.datatype = 'hostname';

		/* could be used also as a default for LLDP, SNMP "system description" in the future */
		o = s.taboption('general', form.Value, 'description', _('Description'), _('An optional, short description for this device'));
		o.optional = true;

		o = s.taboption('general', form.Value, 'location', _('Location'), _('An optional, location of this device'));
		o.optional = true;

		o = s.taboption('general', form.Value, 'contacts', _('Contacts'), _('An optional, contacts of maintaining organization'));
		o.optional = true;

		o = s.taboption('general', form.TextValue, 'notes', _('Notes'), _('Optional, free-form notes about this device'));
		o.optional = true;




		/*
		 * Language & Style
		 */

		o = s.taboption('language', form.ListValue, '_lang', _('Language'))
		o.uciconfig = 'luci';
		o.ucisection = 'main';
		o.ucioption = 'lang';
		o.value('auto');

		var l = Object.assign({ en: 'English' }, uci.get('luci', 'languages')),
			k = Object.keys(l).sort();
		for (var i = 0; i < k.length; i++)
			if (k[i].charAt(0) != '.')
				o.value(k[i], l[k[i]]);


		/*
		 * NTP
		 */

		if (L.hasSystemFeature('sysntpd')) {
			var default_servers = [];

			o = s.taboption('timesync', CBILocalTime, '_systime', _('Local Time'));
			o.cfgvalue = function() { return localtime };
			o.ntpd_support = ntpd_enabled;

			o = s.taboption('timesync', form.ListValue, 'zonename', _('Timezone'));
			//o.value('UTC');

			var zones = Object.keys(timezones);



			for (var i = 0; i < zones.length; i++)
				o.value(zones[i]);
			o.write = function(section_id, formvalue) {
				var tz = timezones[formvalue] ? timezones[formvalue].tzstring : null;
				uci.set('system', section_id, 'zonename', formvalue);
				uci.set('system', section_id, 'timezone', tz);
			};



			o = s.taboption('timesync', form.Flag, 'enabled', _('Enable NTP client'));
			o.rmempty = false;
			o.ucisection = 'ntp';
			o.default = o.disabled;
			o.write = function(section_id, value) {
				ntpd_enabled = +value;

				if (ntpd_enabled && !uci.get('system', 'ntp')) {
					uci.add('system', 'timeserver', 'ntp');
					uci.set('system', 'ntp', 'server', default_servers);
				}

				if (!ntpd_enabled)
					uci.set('system', 'ntp', 'enabled', 0);
				else
					uci.unset('system', 'ntp', 'enabled');

				return callInitAction('sysntpd', 'enable');
			};
			o.load = function(section_id) {
				return (ntpd_enabled == 1 &&
					uci.get('system', 'ntp') != null &&
					uci.get('system', 'ntp', 'enabled') != 0) ? '1' : '0';
			};

			//o = s.taboption('timesync', form.Flag, 'enable_server', _('Provide NTP server'));
			//o.ucisection = 'ntp';
			//o.depends('enabled', '1');

			//o = s.taboption('timesync', widgets.NetworkSelect, 'interface',
			//	_('Bind NTP server'),
			//	_('Provide the NTP server to the selected interface or, if unspecified, to all interfaces'));
			//o.ucisection = 'ntp';
			//o.depends('enable_server', '1');
			//o.multiple = false;
			//o.nocreate = true;
			//o.optional = true;

			o = s.taboption('timesync', form.Flag, 'use_dhcp', _('Use DHCP advertised servers'));
			o.ucisection = 'ntp';
			o.default = o.enabled;
			//o.depends('enabled', '1');

			o = s.taboption('timesync', form.DynamicList, 'server', _('NTP server candidates'));
			o.datatype = 'host(0)';
			o.ucisection = 'ntp';
			//o.depends('enabled', '1');
			o.load = function(section_id) {
				return uci.get('system', 'ntp', 'server');
			};
		}

		return m.render().then(function(mapEl) {
			poll.add(function() {
				return callGetLocaltime().then(function(t) {
					mapEl.querySelector('#localtime').value = formatTime(t);
				});
			});

			return mapEl;
		});
	}
});
