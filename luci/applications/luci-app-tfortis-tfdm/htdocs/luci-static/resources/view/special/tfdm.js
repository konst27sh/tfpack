'use strict';
'require form';
'require view';
'require rpc';
'require ui';
'require network';


var callGetBuiltinEthernetPorts = rpc.declare({
	object: 'luci',
	method: 'getBuiltinEthernetPorts',
	expect: { result: [] }
});



return L.view.extend({
	/** @private */
	render: function(data) {
		var  m, s, o;

		m = new form.Map('network', _('TFortis Device Manager Settings'),	_('Configuration of TFDM settings'));
		s = m.section(form.GridSection, 'interface', _(''),_('You san enable or disable detection through TFortis Device Manager on selected interfaces'));
		s.anonymous = true;
		s.readonly = true;
		s.renderRowActions = function(section_id) {
			var trEdit = this.super('renderRowActions',false);
			return trEdit;
		};
		s.load = function() {
			return Promise.all([
				network.getNetworks(),
			]).then(L.bind(function(data) {
				this.networks = data[0];
			}, this));
		};

		s.cfgsections = function() {
			return this.networks.map(function(n) { return n.getName() })
				.filter(function(n) { return n != 'loopback' });
		};


		o = s.option(form.DummyValue, 'ifn', 'Interface name');
		o.cfgvalue = function(section_id) {
			return section_id;
		};

		o = s.option(form.ListValue, 'tfdm', _('State'));
		o.value("0","Disable");
		o.value("1","Enable");
		o.editable = true;


		return m.render().then(L.bind(function(rendered) {
			return rendered;
		}, this));

	},
	handleReset: null
});
