<?php
function authorizeLogin( $args, $user, $pass ) {
	return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
	return 'SFExtendedControlsDemo';
}


class SFExtendedControlsDemo extends Application
{
	
	public function init( $args ) {
		$this->window->title( 'Container Demo' );
		$this->window->size(640, 480);
		
		$this->window->addChild(
			new XULToolBox(
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
							$check1 = new XULMenuItem('Menuitem5'),
							$check2 = new XULMenuItem('Menuitem6')
						)
					)
					
				)
			),
			new XULToolBox(
				new XULToolbar(
				new XULToolBarButton("New", 'http://www.sitefusion.org/wp/images/examples/filenew.png'),
				new XULToolBarButton("Open", 'http://www.sitefusion.org/wp/images/examples/fileopen.png'),
				new XULToolBarSeparator(),
				$tb1 = new XULToolBarButton("Save", 'http://www.sitefusion.org/wp/images/examples/filesave.png'),
				$tb2 = new XULToolBarButton("Save As", 'http://www.sitefusion.org/wp/images/examples/filesaveas.png')
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
						),
						new XULSplitter(),
						new XULTextBox(strval(""),TRUE)
					),
					$this->panel3 = new XULTabPanel(
						$htmlbox = new XULHtmlBox(1),
						new XULGrid(
							new XULColumns(
								new XULColumn(),
								new XULColumn(),
								new XULColumn()
							),
							new XULRows(
								new XULRow("1")
							)
						)
					),
					$this->panel4 = new XULTabPanel(
						new XULVBox(0,
							new XULImage("http://sitefusion.org/images/header.jpg"),
							new XULTree(1,
								$cols = new XULTreeCols(
									new XULTreeCol('Col1', 1),
									new XULTreeSeparator(),
									new XULTreeCol('Col2', 1),
									new XULSplitter( 'tree-splitter' ),
									new XULTreeCol('Col3', 2)
								),
								$treechildren = new XULTreeChildren(
									new XULTreeItem(
										new XULTreeRow(
											new XULTreeCell('cell1'),
											new XULTreeCell('cell2')
										)
									),
									new XULTreeItem(
										new XULTreeRow(
											new XULTreeCell('cell1'),
											new XULTreeCell('cell2')
										)
									)
								)
							)
						)
					)
				)
			)
		);
		$cols->align('stretch');
		$treechildren->setAttribute('alternatingbackground', 'true');
		$htmlbox->content("<b>this is a bold string</b>");
		$tb1->orient('vertical');
		$tb2->orient('vertical');
		$tb1->align('center');
		$tb2->align('center');
		
		$stylebox
			->setStyle('border', '1px dashed red')
			->setStyle('border', '1px dashed red');
			
		$check1->type("checkbox");
		$check2->type("checkbox");
		
		$check1->setEvent("command", MSG_SEND, $this, "onCheck", $check1);
		$check2->setEvent("command", MSG_SEND, $this, "onCheck", $check2);
	}
	
	public function onCheck($e)
	{
		$this->window->alert($e->sourceObject->checked());	
	}
}