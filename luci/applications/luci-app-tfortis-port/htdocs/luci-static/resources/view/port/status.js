'use strict';
'require baseclass';
'require rpc';
'require form';
'require lldpd';
'require dom';
'require poll';
'require network';
'require fs';
'require uci';

var callNetworkStatus = rpc.declare({
	object: 'luci-rpc',
	method: 'getNetworkDevices',
	expect: {}
});

var callPoeStatus = rpc.declare({
	object: 'poe',
	method: 'info',
	expect: {}
});

var callGetBuiltinEthernetPorts = rpc.declare({
	object: 'luci',
	method: 'getBuiltinEthernetPorts',
	expect: { result: [] }
});

var portlist;

function getState(iface){
	var state = uci.get('port', iface.name, 'state');
	if(state)
		return 'Up';
	else
		return 'Down';
}

function getFlow(iface,view){
	fs.exec('/usr/sbin/ethtool', ["-a", iface]).then(L.bind(function(data) {
		var elem = data.stdout.split(/[\t\n]/);
		if(elem[5] == 'on' && elem[8] == 'on')
			view.innerHTML = 'RX/TX';
		else if(elem[5] == 'on' && elem[8] == 'off')
			view.innerHTML = "RX Only";
		else if(elem[5] == 'off' && elem[8] == 'on')
			view.innerHTML =  "TX Only";
		else
			view.innerHTML = "-";
	}, this));
}

function renderInterfaceLink(data) {
	if(data.up == true)
		return 'Up';
	else
		return 'Down';
}

function renderInterfaceSpeed(speed) {

	if ((typeof speed === 'undefined'))
		return '&#8211;';

	if (speed.link.speed == 10 && speed.link.duplex === 'half')
		return _('10M Half Duplex ');
	if (speed.link.speed == 10 && speed.link.duplex === 'full')
		return _('10M Full Duplex ');
	if (speed.link.speed == 100 && speed.link.duplex === 'half')
		return _('100M Half Duplex ');
	if (speed.link.speed == 100 && speed.link.duplex === 'full')
		return _('100M Full Duplex ');
	if (speed.link.speed == 1000 && speed.link.duplex === 'full')
		return _('1000M Full Duplex ');
	else
		return '&#8211;';
}


function renderInterfaceFlow(flow) {
	if ((typeof flow === 'undefined'))
		return '&#8211;';
	return flow;
}



function renderInterfacePoE(poe) {
	if ((typeof poe === 'undefined'))
		return '&#8211;';
	if (poe.status === "Delivering power")
		return poe.status +' '+ poe.power + 'W';
	else
		return poe.status;
}

function renderStatistics(data) {
	if(data){
		var obj = JSON.parse(JSON.stringify(data));
		if (typeof  obj !== 'undefined') {

			for (var i = 0; i < portlist.length; i++){
				var device = obj[portlist[i].device];

				var flow_view = document.getElementById("flow_"+portlist[i].device);
				if(flow_view != null && device!=null)
					getFlow(device.name,flow_view);

				var view = document.getElementById("status_"+portlist[i].device);
				if(view != null && device!=null)
					view.innerHTML = getState(device);

				view = document.getElementById("link_"+portlist[i].device);
				if(view != null && device!=null)
					view.innerHTML = renderInterfaceLink(device);

				view = document.getElementById("speed_"+portlist[i].device);
				if(view != null && device!=null)
					view.innerHTML = renderInterfaceSpeed(device);
			}
		}else{
			console.log("renderStatistics: fail to parse json");
		}
	}
	else
		console.log("renderStatistics: empty string");
}
function renderPoE(data) {
	if(data){
		var obj = JSON.parse(JSON.stringify(data));
		if (typeof  obj !== 'undefined') {
			let portPoeList = new Array();
			portPoeList = obj.ports;
			for (var i = 0; i < portlist.length; i++){
				var view = document.getElementById("poe_"+portlist[i].device);
				view.innerHTML = renderInterfacePoE(portPoeList[portlist[i].device]);
			}
		}else{
			console.log("renderPoE: fail to parse json");
		}
	}
	else
		console.log("renderPoE: empty string");
}
return L.view.extend({
	renderPortList: function(data) {
		portlist = data;
	},

	startPolling: function() {
		poll.add(L.bind(function() {
			return callNetworkStatus().then(L.bind(function(data) {
				renderStatistics(data);
			}, this));
		}, this),5);
		poll.add(L.bind(function() {
			return callPoeStatus().then(L.bind(function(data) {
				renderPoE(data);
			}, this));
		}, this),5);
	},

	load: function() {
		callGetBuiltinEthernetPorts().then(L.bind(function(data) {
			this.renderPortList(data);
		}, this));
	},


	render: function() {
		var m, s, o;

		m = new form.Map('port');

		s = m.section(form.GridSection, 'lan',_('Port Status'),	_('This page allows you to see port status information'));
		s.anonymous = true;
		s.readonly = true;
		s.editable = false;
		s.renderRowActions = function(section_id) {
			var trEdit = this.super('renderRowActions',false);
			return trEdit;
		};


		o = s.tab('status', _('Status'));

		o = s.taboption('status',form.DummyValue, 'ifn', 'Interface');
		o.cfgvalue = function(section_id) {
			return getPortName(section_id);
		}


		o = s.taboption('status',form.DummyValue, 'status', _('Status'));
		o.editable = true;
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'status_'+section}, _('Collecting data ...'));
		}, this);
		o = s.taboption('status',form.DummyValue, 'link', _('Link'));
		o.editable = true;
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'link_'+section}, _('Collecting data ...'));
		}, this);
		o = s.taboption('status',form.DummyValue, 'speed', _('Speed/Duplex'));
		o.editable = true;
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'speed_'+section}, _('Collecting data ...'));
		}, this);
		o = s.taboption('status',form.DummyValue, 'flow', _('Flow Control'));
		o.editable = true;
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'flow_'+section}, _('Collecting data ...'));
		}, this);
		o = s.taboption('status',form.DummyValue, 'poe', _('PoE'));
		o.editable = true;
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'poe_'+section}, _('Collecting data ...'));
		}, this);

		return m.render().then(L.bind(function(rendered) {
			this.startPolling();
			callNetworkStatus().then(L.bind(function(data) {
				renderStatistics(data);
			}, this));
			callPoeStatus().then(L.bind(function(data) {
				renderPoE(data);
			}, this));
			return rendered;
		}, this));
	},
	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
