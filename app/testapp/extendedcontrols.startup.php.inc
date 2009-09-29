<?php
function authorizeLogin( $args, $user, $pass ) {
	return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
	return 'SFExtendedControlsDemo';
}


class SFExtendedControlsDemo extends Application
{
	
	public function init( $event ) {
		$this->window->title( 'Container Demo' );
		$this->window->size(640, 480);
		
		$this->window->addChild(
			new XULMenuBar(
				new XULMenu('Menu1',
					new XULMenuPopup(
						new XULMenuItem("Menuitem1"),
						new XULMenuItem("Menuitem2")
					)
				),
				new XULMenu('Menu2',
					new XULMenuPopup(
						new XULMenuItem('Menuitem3'),
						new XULMenuItem('Menuitem4')
					)
				),
				new XULMenu('Menu3',
					new XULMenuPopup(
						new XULMenuItem('Menuitem5'),
						new XULMenuItem('Menuitem6')
					)
				)
			),
			new XULTabBox(
				new XULTabs(
					new XULTab("Tab 1"),
					new XULTab("Tab 2"),
					new XULTab("Tab 3"),
					new XULTab("Tab 4")
				),
				new XULTabPanels(
					$this->panel1 = new XULTabPanel(
						new XULGroupBox(1,
							new XULCaption("This is a title"),
							new XULVBox(1,
								new XULRadioGroup(
									new XULRadio('Option 1'),
									new XULRadio('Option 2'),
									new XULRadio('Option 3')
								),
								new XULCheckBox('Additional option 1'),
								new XULCheckBox('Additional option 2')
							)
						)
					),
					$this->panel2 = new XULTabPanel(
						$stylebox = new XULVBox(1,
						new XULLabel('This is a label situated in a XULVBox with some CSS styles applied to it!')
						)
					),
					$this->panel3 = new XULTabPanel(),
					$this->panel4 = new XULTabPanel()
				)
			)
		);
		
		$stylebox
			->setStyle('border', '1px dashed red')
			->setStyle('border', '1px dashed red');
	}
}