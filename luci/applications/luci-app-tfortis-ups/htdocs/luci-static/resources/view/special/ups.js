'use strict';
'require view';
'require fs';
'require ui';
'require rpc';
'require poll';

var callUpsStatus = rpc.declare({
	object: 'tf_hwsys',
	method: 'getCategory',
	params: ['name'],
	expect: { }
});

var ups = E('table', { 'class': 'table' }, [
	E('tr', { 'class': 'tr table-titles' }, [
		E('th', { 'class': 'th col-2 left' }, _('Name')),
		E('th', { 'class': 'th col-10 left' }, _('Value'))
	])
]);

function updateTable(table, data) {
	var rows = [];



	if(typeof data !== 'undefined'){
		var ups_stat = JSON.parse(JSON.stringify(data));
		if (typeof  ups_stat !== 'undefined') {
			if(ups_stat.upsModeAvalible == "1"){
				rows.push([_("UPS subsystem"),_("Ok")]);
				if(ups_stat.upsPwrSource == "1")
					rows.push([_("Power Source"),_("AC")]);
				else
					rows.push([_("Power Source"),_("Battery")]);
				rows.push([_("Battery Voltage"),ups_stat.upsBatteryVoltage+" V"]);
				rows.push([_("Battery Current" ),ups_stat.upsBatCurrent+" mA"]);
				if(ups_stat.upsPwrSource == "1")
					rows.push([_("Charge Voltage"),ups_stat.upsChargeVoltage+" V"]);
				if(ups_stat.upsPwrSource == "0"){
					var date = new Date(0);
					date.setMinutes(ups_stat.upsBatteryTime);
					rows.push([_("Remaining Battery Time"),date.toISOString().substring(11, 19)]);
				}
				rows.push([_("Hw Version"),ups_stat.upsHwVers]);
				rows.push([_("Sw Version"),ups_stat.upsSwVers]);
				rows.push([_("Temperature"),ups_stat.upsRpsTemper+" °C"]);
			}
			else
				rows.push([_("UPS subsystem"),_("Not available")]);


			cbi_update_table(table, rows, E('em', _('No information available')));
		}
	}
}


return view.extend({

	load: function() {
		return L.resolveDefault(callUpsStatus("ups")).then(function(data) {
			updateTable(ups,data)
		});
	},

	startPolling: function() {
		poll.add(L.bind(function() {
			return L.resolveDefault(callUpsStatus("ups")).then(function(data) {
				updateTable(ups,data)
			});
		}, this),10);
	},

	render: function(data) {
		var view = E('div', {'id': 'cbi-ups'}, [
			E('h2', _('UPS Statistics')),

			E('div', { 'class': 'cbi-map-descr' }, _('Detailed information about the UPS module')),

			E('div', {}, [
				E('p', {}, _('')),
				ups
			])
		]);
		this.startPolling();
		return view;
	},
	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
