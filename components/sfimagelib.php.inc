<?php

define("CXIMAGE_FORMAT_BMP", 1);
define("CXIMAGE_FORMAT_GIF", 2);
define("CXIMAGE_FORMAT_JPG", 3);
define("CXIMAGE_FORMAT_PNG", 4);
define("CXIMAGE_FORMAT_ICO", 5);
define("CXIMAGE_FORMAT_TIF", 6);
define("CXIMAGE_FORMAT_TGA", 7);
define("CXIMAGE_FORMAT_PCX", 8);
define("CXIMAGE_FORMAT_WBMP", 9);
define("CXIMAGE_FORMAT_WMF", 10);
define("CXIMAGE_FORMAT_JP2", 11);
define("CXIMAGE_FORMAT_JPC", 12);
define("CXIMAGE_FORMAT_PGX", 13);
define("CXIMAGE_FORMAT_PNM", 14);
define("CXIMAGE_FORMAT_RAS", 15);

define("IM_NEAREST_NEIGHBOUR", 1);
define("IM_BILINEAR", 2);
define("IM_BSPLINE", 3);
define("IM_BICUBIC", 4);
define("IM_BICUBIC2", 5);
define("IM_LANCZOS", 6);
define("IM_BOX", 7);
define("IM_HERMITE", 8);
define("IM_HAMMING", 9);
define("IM_SINC", 10);
define("IM_BLACKMAN", 11);
define("IM_BESSEL", 12);
define("IM_GAUSSIAN", 13);
define("IM_QUADRATIC", 14);
define("IM_MITCHELL", 15);
define("IM_CATROM", 16);

class SFImageLib extends Node
{
	public $remoteConstructor = 'SFImageLib';
	
	
	public function __construct( $url = NULL ) {
		$this->url = $url;
		$this->setEventHandler("stateChange", $this, "onStateChange");
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );

		//$this->createClientEvent( 'createTempPath' );
		//$this->createClientEvent( 'setImageDetails' );
		//$this->createClientEvent ( 'SFImageError' );
		
 		$this->setEvent( 'createTempPath', MSG_SEND, $this, 'createTempPath' );
 		$this->setEvent( 'setImageDetails', MSG_SEND, $this, 'setImageDetails' );
 		
		$this->setEventReflex( 'createTempPath', 'eventArguments.push( '.$this->js.'.getTempFilePath());' );
	}
	
	public function getTempFilePath($format)
	{
		switch ($format)
		{
			case CXIMAGE_FORMAT_JPG:
			$extension = ".jpg";
			break;
			case CXIMAGE_FORMAT_PNG:
			$extension = ".png";
			break;
			case CXIMAGE_FORMAT_GIF:
			$extension = ".gif";
			break;
			default:
			$extension = ".jpg";
			break;
		}
		$this->tempFilePath = $this->tempFilePath.$extension;
		$this->setProperty( 'temppath', $this->tempFilePath );
		return $this->tempFilePath;
	}
	
	public function cleanTemp()
	{
		if ($this->tempFilePath)
		{
			$this->callMethod( 'cleanTemp' );
		}
	}
	
	public function createTempPath ($e, $temppath)
	{
		$this->tempFilePath = $temppath;
	}
	
	public function loadImage($path)
	{
		$this->path = $path;
		$this->height = NULL;
		$this->width = NULL;
		$this->format = NULL;
		$this->tempFilePath = NULL;
		$this->setProperty( 'path', $this->path );
		$this->callMethod( 'loadImage', array( $path  ) );
	}
	
	public function saveImage($path, $type)
	{
		$this->callMethod( 'saveImage', array( $path, $type  ) );
	}
	
	public function resample($width, $height, $mode)
	{
		$this->callMethod( 'resample', array( $width, $height, $mode  ) );
	}
	
	public function setImageDetails($e, $localevent, $width, $height, $format)
	{
		$this->width = $width;
		$this->height = $height;
		$this->format = $format;
		
		$this->fireLocalEvent($localevent);
	}
	
	public function getWidth()
	{
		return $this->width;
	}
	
	public function getHeight()
	{
		return $this->height;
	}
	
	public function getFormat()
	{
		return $this->format;
	}
	
	public function getPath()
	{
		return $this->path;
	}
	
}

class WPDialogClientImageSizer extends XULDialog
{
	public $modal = TRUE;
	
	public function __construct() {
		$this->title = WPLanguage::$ImageEditor_ResizeDialog_Title;
		
		$this->addChild(
				new XULHBox(
				new XULVBox('center',
						$this->imagePreview = new XULImage()
					),
				new XULVBox(
				new XULGroupBox(
				new XULCaption( WPLanguage::$ImageEditor_ResizeDialog_CurrentSize, 'bold' ),
					new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn(1)
						),
						new XULRows(
							new XULRow(
								new XULLabel( WPLanguage::$ImageEditor_ResizeDialog_Width ),
								$this->lblWidth = new XULLabel()
							),
							new XULRow(
								new XULLabel( WPLanguage::$ImageEditor_ResizeDialog_Height ),
								$this->lblHeight = new XULLabel()
							)
						)
						)
					),
					new XULGroupBox(
						new XULCaption( WPLanguage::$ImageEditor_ResizeDialog_NewSize, 'bold' ),
						
						new XULGrid(
							new XULColumns(
								new XULColumn,
								new XULColumn,
								new XULColumn
							),
							new XULRows(
								new XULRow( 'center',
									new XULLabel( WPLanguage::$ImageEditor_ResizeDialog_Width ),
									$this->wInp = new XULTextBox( (string) "", FALSE, 50 ),
									$this->wUnit = new XULMenuList(
										new XULMenuPopup(
											new XULMenuItem( WPLanguage::$ImageEditor_ResizeDialog_Pixels, 'px' ),
											new XULMenuItem( WPLanguage::$ImageEditor_ResizeDialog_Percent, '%' )
										)
									)
								),
								new XULRow( 'center',
									new XULLabel( WPLanguage::$ImageEditor_ResizeDialog_Height ),
									$this->hInp = new XULTextBox( (string) "", FALSE, 50 ),
									$this->hUnit = new XULMenuList(
										new XULMenuPopup(
											new XULMenuItem( WPLanguage::$ImageEditor_ResizeDialog_Pixels, 'px' ),
											new XULMenuItem( WPLanguage::$ImageEditor_ResizeDialog_Percent, '%' )
										)
									)
								),
								$this->ckProportions = new XULCheckBox( WPLanguage::$ImageEditor_ResizeDialog_MaintainProportions, TRUE )
							)
						)
					)
				)
			)
		);
		
		$this->wInp->type( 'number' )->setAttribute( 'min', '1' )->setAttribute( 'max', '4000' );
		$this->hInp->type( 'number' )->setAttribute( 'min', '1' )->setAttribute( 'max', '4000' );
		
		$this->setEvent( 'accept', MSG_SEND, $this, 'accept', array( $this->wInp, $this->hInp, $this->wUnit, $this->hUnit ) );
	}
	
	public function setReflexes($sfimageObj)
	{
		$wi = $this->wInp;
		$hi = $this->hInp;
		$wu = $this->wUnit;
		$hu = $this->hUnit;
		$cp = $this->ckProportions;
		$w = $sfimageObj->getWidth();
		$h = $sfimageObj->getHeight();
		
		$this->wInp->setEventReflex( "change",
			"
			var v = $wi->js.element.valueNumber;
			
			if( $cp->js.element.checked )
				$hi->js.element.valueNumber = $wu->js.element.selectedIndex ? v : parseInt( $h * (v / $w) );
			"
		);
		
		$this->hInp->setEventReflex( "change",
			"
			var v = $hi->js.element.valueNumber;
			
			if( $cp->js.element.checked )
				$wi->js.element.valueNumber = $wu->js.element.selectedIndex ? v : parseInt( $w * (v / $h) );
			"
		);
		
		$this->wUnit->setEventReflex( "command",
			"
			var u = $hu->js.element.selectedIndex = $wu->js.element.selectedIndex;
			if( u ) $wi->js.element.valueNumber = $hi->js.element.valueNumber = 100;
			"
		);
		
		$this->hUnit->setEventReflex( "command",
			"
			var u = $wu->js.element.selectedIndex = $hu->js.element.selectedIndex;
			if( u ) $wi->js.element.valueNumber = $hi->js.element.valueNumber = 100;
			"
		);	
	}
	
	public function accept( $e ) {
		$w = $this->wInp->value();
		$h = $this->hInp->value();
		
		if( $this->wUnit->selectedValue == '%' ) {
			$w = (int) ($this->image->getWidth() * ($w / 100));
			$h = (int) ($this->image->getHeight() * ($h / 100));
		}
		
		$this->fireLocalEvent( 'sizeAccepted', array( $w, $h ) );
	}
	
	public function init() {
		
	}
	
	public function loadImage($sfimageObj)
	{
		$maxDimension = max($sfimageObj->getWidth(), $sfimageObj->getHeight());
		$targetmax = 300;
		
		$this->imagePreview->src("file://".$sfimageObj->path);
		
		$this->wInp->value((string)$sfimageObj->getWidth());
		$this->hInp->value((string)$sfimageObj->getHeight());
		 
		$this->lblWidth->value($sfimageObj->getWidth() . ' ' . WPLanguage::$ImageEditor_ResizeDialog_Pixels);
		$this->lblHeight->value($sfimageObj->getHeight() . ' ' . WPLanguage::$ImageEditor_ResizeDialog_Pixels);
		 
		if ($maxDimension > $targetmax)
		{
			$factor = $targetmax / $maxDimension;
			$image->width(intval($factor * $sfimageObj->getWidth()));
			$image->height(intval($factor * $sfimageObj->getHeight()));
		}
	}
	
	static public function Open( $mediaBrowser, $sfimageObj ) {
		if( ! isset($app->codeEditor) ) {
			$mediaBrowser->clientImageSizer = new self;
			$mediaBrowser->addChild( $mediaBrowser->clientImageSizer );
		}
		
		$mediaBrowser->clientImageSizer->loadImage( $sfimageObj);
	}
}