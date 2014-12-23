<?PHP
// =============================================================================
// GIANA Framework | Home Automation Made Easy. (LAMP || WAMP) + Arduino UNO r3.
// =============================================================================
// Copyright (C) 2013 Federico Pfaffendorf (www.federicopfaffendorf.com.ar)
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version. 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/gpl.txt
// =============================================================================
// Please, use it under PHP 5 or greater.
// =============================================================================
include_once("inc/php_serial.class.php");
// =============================================================================
// CLASS Context
// =============================================================================
class Context
{
	
	// ===========================================================================
	// Properties
	// ===========================================================================

	public static $gianas = array();

  // -------------------------------------------------------------------------
  // Change this to match your services root URI
  // -------------------------------------------------------------------------
	const servicesRootURI = "http://localhost/giana/srv/";				// |
	// -------------------------------------------------------------------------		
	
  // -------------------------------------------------------------------------
  // Change this random string, here and in your security.config.php
  // -------------------------------------------------------------------------
	const SALT = "0G9UFVc8VQTCs5qosA6g";																		// |
	// -------------------------------------------------------------------------		

  // -------------------------------------------------------------------------
  // Change this match your Arduino baud rate
  // -------------------------------------------------------------------------
	const BaudRate = 9600;																									// |
	// -------------------------------------------------------------------------		
	
	// ===========================================================================
	// Methods
	// ===========================================================================

	public static function initialize()
	{

		// -------------------------------------------------------------------------
    // Add as many Gianas as Arduinos you have !
    // -------------------------------------------------------------------------
		Context::$gianas[] = new Giana("/dev/ttyACM0");													// |
		// -------------------------------------------------------------------------		
		
	}
	
	public static function getServicesRootURI()
	{
	
		return Context::servicesRootURI .
			(substr (Context::servicesRootURI, 
							strlen(Context::servicesRootURI) - 1, 1) == "/" ? "" : "/");
							
	}
	
	public static function getGianaAt($port)
	{
		foreach(Context::$gianas as $giana)
			if ($giana->port == $port)
				return $giana;
		Log::this("There is no Giana at port " . $port . ".", false);
		return null;
	}
	
}
// =============================================================================
// MAIN
// =============================================================================
{

	// =============================================================================
	Context::initialize();
	// =============================================================================

	// =============================================================================
	Log::this("Program started.", false);
	// =============================================================================

	// =============================================================================
	// Schedule
	// =============================================================================
	$schedule = Schedule::get();
	if ($schedule != null)
	{
			$giana = Context::getGianaAt($schedule->port);
			if ($giana != null)
					$giana->performSchedule($schedule);
			else
					$schedule->done(true);
	}	
	// =============================================================================
	// Request
	// =============================================================================
	$request = Request::get();
	if ($request != null)
	{
		$giana = Context::getGianaAt($request->port);
		if ($giana != null)
				$giana->performRequest($request);
		else
		{
			$response = new Response(
					$request->id,
					Response::Error
				);
			$response->post();
		}
	}
	// =============================================================================

	// =============================================================================
	sleep(1);	
	// =============================================================================
	
	// =============================================================================
	Log::this("Program ended.", false);
	// =============================================================================

}
// =============================================================================
// CLASS Giana
// =============================================================================
class Giana 
{

	// ===========================================================================
	// Properties
	// ===========================================================================

	public $port;

	// ===========================================================================
	// Constructors
	// ===========================================================================
	
	function __construct($port)
	{
		$this->port = $port;
	}
	
	// ===========================================================================
	// Public Methods
	// ===========================================================================

	public function performSchedule($schedule)
	{
	
		$value = $this->scheduleConditions($schedule->conditions);
		
		$request = new Request (
			null, $schedule->port, Action::Set, $schedule->pinType,
			$schedule->pinNumber, 
			$value ? $schedule->trueValue : $schedule->falseValue
		);
		
		set($request);
		
		$schedule->done(false);
	
	}
	
	public function performRequest($request)
	{

		$response = null;
	
		if ($request->action == Action::Get)
			$response = $this->get($request);
		else
			$response = $this->set($request);

		if ($response != null)
			$response->post();
		else
		{
			$response = new Response(
					$request->id,
					Response::Error
				);
			$response->post();
		}	
			
	}
	
	// ===========================================================================
	// Private Methods
	// ===========================================================================
	
	private function scheduleConditions($conditions)
	{
	
		foreach ($conditions as $condition)
		{
		
			$request = new Request (
				null, $condition->port, Action::Get, $condition->pinType,
				$condition->pinNumber, 0				
			);
			
			$response = get($request);
			
			switch($condition->sign)
			{
			
				case '>':
						if ($response->value < $condition->rawValue)
							return false;
					break;
			
				case '<':
						if ($response->value > $condition->rawValue)
							return false;
					break;
			
				case '=':
						if ($response->value != $condition->rawValue)
							return false;
					break;
			
			}
		
		}
	
		return true;
	
	}
	
	private function get($request)
	{
	
		$message = "<G";
		$message .= $request->pinType;
		if ($request->pinNumber < 10) $message .= "0";
		$message .= $request->pinNumber;
		$message .= ">";

		$serial = new phpSerial();
		$serial->deviceSet($request->port); 
		$serial->confBaudRate(Context::BaudRate); 		
		$serial->deviceOpen(); 
		$serial->sendMessage($message); 

		sleep(1);

		$message = $serial->readPort(); 

		$serial->deviceClose(); 
		
		$response = new Response (
				$request->id,
				(int)substr($message, 4, ($request->pinType == PinType::Digital ? 1 : 4))
			);
	
		return $response;
	
	}

	private function set($request)
	{
	
		$message = "<SD";
		if ($request->pinNumber < 10) $message .= "0";
		$message .= $request->pinNumber;
		$message .= $request->value;
		$message .= ">";
		
		$serial = new phpSerial();
		$serial->deviceSet($request->port); 
		$serial->confBaudRate(Context::BaudRate); 		
		$serial->deviceOpen(); 
		
		$serial->sendMessage($message); 
		
		sleep(1);

		$message = $serial->readPort(); 

		$serial->deviceClose(); 
	
		$response = new Response (
				$request->id,
				substr($message, 6, 2)
			);
	
		return $response;
	
	}
	
}
// =============================================================================
// CLASS Schedule
// =============================================================================
class Schedule 
{

	// ===========================================================================
	// Properties
	// ===========================================================================

	public $id;
	public $port;
	public $pinType;
	public $pinNumber;
	public $trueValue;
	public $falseValue;
	public $conditions = array();

	// ===========================================================================
	// Methods
	// ===========================================================================

	public static function get()
	{

		$domDocument;
	
		try {
		
			$domDocument =		
				REST::this(
					Context::getServicesRootURI() . "schedule/", 
					"GET");
				
		} 
		catch (Exception $exception)
		{
		
			Log::this($exception->getMessage(), true);
			return null;
			
		}

		if ($domDocument->getElementsByTagName("device")->length > 0)
		{
		
			$schedule = new Schedule();
		
			$schedule->id = $domDocument->getElementsByTagName("schedule")->item(0)->getAttribute("id");
			$schedule->port = $domDocument->getElementsByTagName("port")->item(0)->nodeValue;
			$schedule->pinType = $domDocument->getElementsByTagName("pinType")->item(0)->nodeValue;
			$schedule->pinNumber = $domDocument->getElementsByTagName("pinNumber")->item(0)->nodeValue;
			$schedule->trueValue = $domDocument->getElementsByTagName("trueValue")->item(0)->nodeValue;
			$schedule->falseValue = $domDocument->getElementsByTagName("falseValue")->item(0)->nodeValue;
		
			return $schedule;
		
		}
		
		return null;
	
	}
	
	public function done($error)
	{
	
		try {
		
			REST::this(
				Context::getServicesRootURI() . "schedule/?scheduleId=" . $this->id .
				"&error=" . ($error ? "Y" : "N"), 
				"POST");
				
		} 
		catch (Exception $exception)
		{
		
			Log::this($exception->getMessage(), true);
			
		}
		
	}
	
}
// =============================================================================
// CLASS Condition
// =============================================================================
class Condition
{

	// ===========================================================================
	// Properties
	// ===========================================================================

	public $port;
	public $pinType;
	public $pinNumber;
	public $sign;
	public $rawValue;

	// ===========================================================================
	// Constructors
	// ===========================================================================

	function __construct ($port, $pinType, $pinNumber, $sign, $rawValue)
	{
	
		$this->port = $port;
		$this->pinType = $pinType;
		$this->pinNumber = $pinNumber;
		$this->sign = $sign;
		$this->rawValue = $rawValue;
	
	}
	
}
// =============================================================================
// CLASS Request
// =============================================================================
class Request
{

	// ===========================================================================
	// Properties
	// ===========================================================================
	
	public $id;
	public $port;
	public $action;
	public $pinType;
	public $pinNumber;
	public $value;

	// ===========================================================================
	// Constructors
	// ===========================================================================
	
	function __construct($id, $port, $action, $pinType, $pinNumber, $value)
	{
	
		$this->id = $id;
		$this->port = $port;
		$this->action = $action;
		$this->pinType = $pinType;
		$this->pinNumber = $pinNumber;
		$this->value = $value;
		
	}

	// ===========================================================================
	// Public Methods
	// ===========================================================================
	
	public static function get()
	{
	
		$domDocument;
	
		try {
		
			$domDocument =
				REST::this(
					Context::getServicesRootURI() . "request/", "GET");
				
		} 
		catch (Exception $exception)
		{
		
			Log::this($exception->getMessage(), true);
			return null;
			
		}
	
		if ($domDocument->getElementsByTagName("action")->length > 0)
		{
		
			$request = new Request(
				$domDocument->getElementsByTagName("requestId")->item(0)->nodeValue,
				$domDocument->getElementsByTagName("port")->item(0)->nodeValue,
				$domDocument->getElementsByTagName("action")->item(0)->nodeValue,
				$domDocument->getElementsByTagName("pinType")->item(0)->nodeValue,
				$domDocument->getElementsByTagName("pinNumber")->item(0)->nodeValue,
				$domDocument->getElementsByTagName("value")->item(0)->nodeValue
			);
		
			return $request;
		
		}
	
		return null;
	
	}
	
}
// =============================================================================
// CLASS Response
// =============================================================================
class Response
{
	
	// ===========================================================================
	// Properties
	// ===========================================================================
	
	public $requestId;
	public $value;
	
	const Error = "ER";
	const Ok = "OK";
	
	// ===========================================================================
	// Constructors
	// ===========================================================================

	function __construct($requestId, $value)
	{
		$this->requestId = $requestId;
		$this->value = $value;
	}

	// ===========================================================================
	// Public Methods
	// ===========================================================================

	public function post()
	{

		try {

			REST::this(
				Context::getServicesRootURI() . 
				"response/?requestId=" . $this->requestId .
				"&value=" . $this->value, "POST");
																	 
		} 
		catch (Exception $exception)
		{
		
			Log::this($exception->getMessage(), true);
		
		}
																	 
	}
	
}
// =============================================================================
// CLASS REST
// =============================================================================
class REST
{

	// =========================================================================
	// Public Methods
	// =========================================================================

	public static function this($uri, $method)
	{
	
		if (strpos($uri, "?") === false)
			$uri .= "?SALT=" . Context::SALT;
	  else
			$uri .= "&SALT=" . Context::SALT;

		$curl = curl_init();			
		
		switch ($method)
    {
      case "POST":
				curl_setopt($curl, CURLOPT_POST, 1);
				curl_setopt($curl, CURLOPT_POSTFIELDS, "");
				break;
      case "PUT":
          curl_setopt($curl, CURLOPT_PUT, 1);
				break;
    }
		
		curl_setopt($curl, CURLOPT_URL, $uri);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);

		$domDocument = new DomDocument();
		//$respuesta=curl_exec($curl);
		//echo $respuesta;
		//$domDocument->loadXML($respuesta);
		$domDocument->loadXML(curl_exec($curl));
		
    return $domDocument;
		
	}

}
// =============================================================================
// CLASS Log
// =============================================================================
class Log
{

	// =========================================================================
	// Public Methods
	// =========================================================================

	public static function this($message, $exception)
	{
		echo(
			date("Y-m-d H:i:s") . " | " . ($exception ? "[EXCEPTION]" : "") . 
			$message . "\r\n"
		);
	}

}
// =============================================================================
// CLASS Action
// =============================================================================
class Action
{
	const Get = "G";
	const Set = "S";
}
// =============================================================================
// CLASS PinType
// =============================================================================
class PinType
{
	const Analogue = "A";
	const Digital = "D";
}
// =============================================================================
?>