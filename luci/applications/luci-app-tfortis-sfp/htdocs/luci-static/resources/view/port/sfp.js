'use strict';
'require form';
'require fs';
'require view';
'require rpc';
'require poll';
'require uci';
'require ui';

var callSfpStatus = rpc.declare({
	object: 'tf_hwsys',
	method: 'getCategory',
	params: ['name'],
	expect: {  }
});

var sfp1 = E('table', { 'class': 'table' }, [
	E('tr', { 'class': 'tr table-titles' }, [
		E('th', { 'class': 'th col-2 left' }, _('Name')),
		E('th', { 'class': 'th col-10 left' }, _('Value'))
	])
]);
var sfp2 = E('table', { 'class': 'table' }, [
	E('tr', { 'class': 'tr table-titles' }, [
		E('th', { 'class': 'th col-2 left' }, _('Name')),
		E('th', { 'class': 'th col-10 left' }, _('Value'))
	])
]);

function updateTable(table, data, port) {
	var rows = [];
	console.log(data);
	var sfp = JSON.parse(JSON.stringify(data));
	if (typeof  sfp !== 'undefined') {
		if(port == 0){
			rows.push([_("Present"),sfp.portSfpPresent_1]);
			rows.push([_("Los"),sfp.portSfpSignalDetect_1]);
			rows.push([_("Vendor"),sfp.portSfpVendor_1]);
			rows.push([_("Vendor OUI"),sfp.portSfpOui_1]);
			rows.push([_("Vendor PN"),sfp.portSfpPartNumber_1]);
			rows.push([_("Vendor REV"),sfp.portSfpRevision_1]);
			rows.push([_("Identifier"),sfp.portSfpIdentifier_1]);
			rows.push([_("Connector"),sfp.portSfpConnector_1]);
			rows.push([_("Type"),sfp.portSfpType_1]);
			rows.push([_("Link length"),sfp.portSfpLinkLen_1]);
			rows.push([_("Fiber technology"),sfp.portSfpFiberTec_1]);
			rows.push([_("Media"),sfp.portSfpMedia_1]);
			rows.push([_("Speed"),sfp.portSfpSpeed_1]);
			rows.push([_("Encoding"),sfp.portSfpEncoding_1]);
			rows.push([_("Wave length"),sfp.portSfpWavelen_1]);
			rows.push([_("NBR"),sfp.portSfpNBR_1]);
			rows.push([_("len9"),sfp.portSfpLen9_1]);
			rows.push([_("len50"),sfp.portSfpLen50_1]);
			rows.push([_("len62"),sfp.portSfpLen62_1]);
			rows.push([_("lenc"),sfp.portSfpLenC_1]);
			rows.push([_("Temperature"),sfp.portSfpTemperature_1]);
			rows.push([_("Voltage"),sfp.portSfpVoltage_1]);
			rows.push([_("Current"),sfp.portSfpCurrent_1]);
			rows.push([_("TX Bias"),sfp.portSfpBiasCurrent_1]);
			rows.push([_("TX Power (Db)"),sfp.portSfpTxOutPowerDb_1]);
			rows.push([_("TX Power (mW)"),sfp.portSfpTxOutPower_1]);
			rows.push([_("RX Power (Db)"),sfp.portSfpRxOutPowerDb_1]);
			rows.push([_("RX Power (mW)"),sfp.portSfpRxOutPower_1]);
		}
		if(port == 1){
			rows.push([_("Present"),sfp.portSfpPresent_2]);
			rows.push([_("Los"),sfp.portSfpSignalDetect_2]);
			rows.push([_("Vendor"),sfp.portSfpVendor_2]);
			rows.push([_("Vendor OUI"),sfp.portSfpOui_2]);
			rows.push([_("Vendor PN"),sfp.portSfpPartNumber_2]);
			rows.push([_("Vendor REV"),sfp.portSfpRevision_2]);
			rows.push([_("Identifier"),sfp.portSfpIdentifier_2]);
			rows.push([_("Connector"),sfp.portSfpConnector_2]);
			rows.push([_("Type"),sfp.portSfpType_2]);
			rows.push([_("Link length"),sfp.portSfpLinkLen_2]);
			rows.push([_("Fiber technology"),sfp.portSfpFiberTec_2]);
			rows.push([_("Media"),sfp.portSfpMedia_2]);
			rows.push([_("Speed"),sfp.portSfpSpeed_2]);
			rows.push([_("Encoding"),sfp.portSfpEncoding_2]);
			rows.push([_("Wave length"),sfp.portSfpWavelen_2]);
			rows.push([_("NBR"),sfp.portSfpNBR_2]);
			rows.push([_("len9"),sfp.portSfpLen9_2]);
			rows.push([_("len50"),sfp.portSfpLen50_2]);
			rows.push([_("len62"),sfp.portSfpLen62_2]);
			rows.push([_("lenc"),sfp.portSfpLenC_2]);
			rows.push([_("Temperature"),sfp.portSfpTemperature_2]);
			rows.push([_("Voltage"),sfp.portSfpVoltage_2]);
			rows.push([_("Current"),sfp.portSfpCurrent_2]);
			rows.push([_("TX Bias"),sfp.portSfpBiasCurrent_2]);
			rows.push([_("TX Power (Db)"),sfp.portSfpTxOutPowerDb_2]);
			rows.push([_("TX Power (mW)"),sfp.portSfpTxOutPower_2]);
			rows.push([_("RX Power (Db)"),sfp.portSfpRxOutPowerDb_2]);
			rows.push([_("RX Power (mW)"),sfp.portSfpRxOutPower_2]);
		}
	}


	cbi_update_table(table, rows, E('em', _('No information available')));
};


return L.view.extend({


	load: function() {
		L.resolveDefault(callSfpStatus("sfp1")).then(function(data) {
			updateTable(sfp1, data,0);
		});
		L.resolveDefault(callSfpStatus("sfp2")).then(function(data) {
			updateTable(sfp2, data,1);
		});
	},





	render: function(data) {
		var view = E('div', {'id':'cbi-network','class':'cbi-map'}, [
			E('div', { 'class': 'cbi-section' },[

				E('h3', _('SFP Statistics')),
				E('div', { 'class': 'cbi-map-descr' }, _('Detailed information about the connected SFP module(DDM, manufactor, type, etc).')),

				E('div', { }, [
					E('div', { 'data-tab': 'init', 'data-tab-title': _('SFP1') }, [
						E('p', {}, _('')),
						sfp1
					]),
					E('div', { 'data-tab': 'init2', 'data-tab-title': _('SFP2') }, [
						E('p', {}, _('')),
						sfp2
					])
				])
			])

		]);
		return view;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
