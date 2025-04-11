'use strict';
'require form';
'require fs';
'require view';
'require rpc';
'require poll';
'require uci';
'require ui';

var mdb_table = E('table', { 'class': 'table' }, [
	E('tr', { 'class': 'tr table-titles' }, [
		E('th', { 'class': 'th col-0 left' }, _('')),
		E('th', { 'class': 'th col-2 left' }, _('Group')),
		E('th', { 'class': 'th col-2 left' }, _('Port')),
		E('th', { 'class': 'th col-10 left' }, _('VLAN'))
	])
]);


function getMDBTable() {
	return fs.exec_direct("/usr/share/lua/mdb_module.lua",['getJson']).then(function(res){
		return res;
	});
}


function updateTable(table, data) {
	var rows = [];

	var json = JSON.parse(data);
	if (typeof  json !== 'undefined') {

		console.log(json);
		console.log(json.length);

		for(var i=0; i<json.length; i++){
			//console.log(json.at());
			rows.push([i+1,json[i].grp,getPortName(json[i].port),json[i].vid]);
		}
		//rows.push([_("Present"),json.portSfpPresent_1]);
		//rows.push([_("Los"),json.portSfpSignalDetect_1]);
	}
	cbi_update_table(table, rows, E('em', _('No information available')));
};


return L.view.extend({


	load: function() {
		getMDBTable().then(function(data) {
			updateTable(mdb_table, data);
		});

	},





	render: function(data) {
		var view = E('div', {'id':'cbi-network','class':'cbi-map'}, [
			E('div', { 'class': 'cbi-section' },[

				E('h3', _('IGMP Statistics')),
				E('div', { 'class': 'cbi-map-descr' }, _('Detailed information about IGMP groups')),

				E('div', { }, [
					//E('div', { 'data-tab': 'init', 'data-tab-title': _('SFP1') }, [
					E('p', {}, _('')),
					mdb_table
					//])

				])
			])

		]);
		return view;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
