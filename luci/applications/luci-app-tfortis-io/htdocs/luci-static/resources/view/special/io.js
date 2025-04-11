'use strict';
'require form';
'require fs';
'require view';
'require rpc';
'require poll';
'require uci';


var callGetSensorStatus = rpc.declare({
	object: 'tf_hwsys',
	method: 'getCategory',
	params: ['name'],
	expect: { }
});

var callSetRelayState = rpc.declare({
	object: 'tf_hwsys',
	method: 'setParam',
	params: ["name","value"],
	expect: { }
});

function callManualState(section_id, state){
	if(state == "open")
		callSetRelayState("relay","0");
	else
		callSetRelayState("relay","1");
}

function renderStatus(data) {
	if(data == 1)
		return _("Short");
	else
		return _("Open");
}

function renderStatistics(data) {
	var view;
	if(data){
		const obj = JSON.parse(JSON.stringify(data));
		if (typeof  obj !== 'undefined') {
			//tamper
			view = document.getElementById("status_tamper");
			if(view != null && obj.tamper!=null)
				view.innerHTML = renderStatus(obj.tamper);

			//sensor1
			view = document.getElementById("status_sensor1");
			if(view != null && obj.sensor1!=null)
				view.innerHTML = renderStatus(obj.sensor1);

			//sensor2
			view = document.getElementById("status_sensor2");
			if(view != null && obj.sensor2!=null)
				view.innerHTML = renderStatus(obj.sensor2);

			//relay
			view = document.getElementById("status_relay");
			if(view != null && obj.relay!=null)
				view.innerHTML = renderStatus(obj.relay);

			//sensor connected
			view = document.getElementById("sensorConnected");
			if(view != null && obj.sensorConnected!=null){
				if(obj.sensorConnected == 1)
					view.innerHTML = _("Connected");
				else
					view.innerHTML = _("Not connected");
			}

			//temper
			view = document.getElementById("sensorTemperature");
			if(view != null && obj.sensorTemperature!=null && obj.sensorConnected)
				view.innerHTML = obj.sensorTemperature;

			//humid
			view = document.getElementById("sensorHumidity");
			if(view != null && obj.sensorHumidity!=null && obj.sensorConnected)
				view.innerHTML = obj.sensorHumidity;




		}else{
			console.log("renderStatistics: fail to parse json");
		}
	}
	else
		console.log("renderStatistics: empty string");
}


return L.view.extend({
	/** @private */
	load: function() {
		return L.resolveDefault(callGetSensorStatus("sensors")).then(function(data) {
			renderStatistics(data);
		});
	},
	startPolling: function() {
		poll.add(L.bind(function() {
			return L.resolveDefault(callGetSensorStatus("sensors")).then(function(data) {
				renderStatistics(data);
			});
		}, this),5);
	},


	render: function(data) {
		var  m, s, o;

		m = new form.Map('tfortis_io', _('Inputs/Outputs'),	_('Configuration of build-in inputs/outputs'));
		s = m.section(form.GridSection, 'input', _('Inputs'));
		s.anonymous = false;
		s.readonly = true;
		s.renderRowActions = function(section_id) {
			var trEdit = this.super('renderRowActions',false);
			return trEdit;
		};


		o = s.option(form.Flag, 'state', _('Activated'));
		o.editable = true;


		o = s.option(form.ListValue, 'alarm_state', _('Alarm state'));
		o.value("open",_("Open"));
		o.value("short",_("Short"));
		o.editable = true;


		o = s.option(form.DummyValue, 'status', _('Current state'));
		o.editable = true;

		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'status_'+section}, _('Collecting data ...'));
		}, this);



		s = m.section(form.GridSection, 'output', _('Outputs'));
		s.anonymous = false;
		s.readonly = true;
		s.renderRowActions = function(section_id) {
			var trEdit = this.super('renderRowActions',false);
			return trEdit;
		};


		o = s.option(form.ListValue, 'state', _('State'));
		o.value("open",_("Open"));
		o.value("short",_("Short"));
		o.width = '20%';
		o.editable = true;
		o.onchange = function(ev, section, value) {
			callManualState(section, value);
		};


		o = s.option( form.DummyValue, 'status', _('Current state'));
		o.editable = true;
		o.width = '50%';
		o.modalonly = false;
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'status_'+section}, _('Collecting data ...'));
		}, this);


		s = m.section(form.GridSection , 'sht', _('Temperature sensor'));
		s.anonymous = false;
		s.readonly = true;
		s.renderRowActions = function(section_id) {
			var trEdit = this.super('renderRowActions',false);
			return trEdit;
		};



		o = s.option(form.DummyValue, 'connected',_('Status'));
		o.editable = true;
		o.width = '20%';
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'sensorConnected'}, _('Collecting data ...'));
		}, this);


		o = s.option(form.DummyValue, 'sensorTemperature', _('Temperature'));
		o.editable = true;
		o.width = '20%';
		o.modalonly = false;
		o.depends('connected', '1');
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'sensorTemperature'}, _('Collecting data ...'));
		}, this);

		o = s.option(form.DummyValue, 'sensorHumidity', _('Humidity'));
		o.editable = true;
		o.width = '20%';
		o.modalonly = false;
		o.depends('connected', '1');
		o.render = L.bind(function(self, section, scope) {
			return 	E('td', { class: 'td cbi-value-field', id: 'sensorHumidity'}, _('Collecting data ...'));
		}, this);


		return m.render().then(L.bind(function(rendered) {
			this.startPolling();
			return rendered;
		}, this));
	}
});
