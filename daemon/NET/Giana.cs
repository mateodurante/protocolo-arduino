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
// Please, compile using .NET Framework 2.0 or greater.
// =============================================================================
using System;
using System.IO;
using System.Xml;
using System.Net;
using System.Text;
using System.IO.Ports;
using System.Threading;
using System.Collections.Generic;
using System.Collections.ObjectModel;
// =============================================================================
// CLASS Context
// =============================================================================
static class Context
{

    #region Properties

    // -------------------------------------------------------------------------
    // Add as many Gianas as Arduinos you have !
    // -------------------------------------------------------------------------
    private static Giana[] Gianas = new Giana[] {                           // |
                                        new Giana(                          // |
                                            new SerialPort("COM13", 9600)   // |
                                        )                                   // |
                                    };                                      // |
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Change this to match your services root URI
    // -------------------------------------------------------------------------
    private static string _ServicesRootURI =                                // |
        "http://localhost:8888/giana/srv/";                                 // |
    // -------------------------------------------------------------------------

    public static string ServicesRootURI
    {
        get
        {
            return _ServicesRootURI + (
                _ServicesRootURI.EndsWith("/") ? "" : "/"
                );
        }
    }

    // -------------------------------------------------------------------------
    // Change this random string, here and in your security.config.php
    // -------------------------------------------------------------------------
    private static string _SALT =                                           // |
        "0G9UFVc8VQTCs5qosA6g";                                             // |
    // -------------------------------------------------------------------------

    public static string SALT
    {
        get
        {
            return _SALT;
        }
    }

    #endregion

    #region Methods

    public static Giana GetGianaAt(string port)
    {
        foreach (Giana giana in Gianas)
            if (giana.Port == port)
            {
                return giana;
            }
        Log.This("There is no Giana at port " + port + ".");
        return null;
    }

    #endregion

}
// =============================================================================
// CLASS Program
// =============================================================================
class Program
{

    #region Main

    public static void Main()
    {

        Log.This("Program started.");

        while (true)
        {

            Schedule schedule;
            Request request;
            Giana giana;

            #region Schedule

            schedule = Schedule.Get();

            if (schedule != null)
            {
                giana = Context.GetGianaAt(schedule.Port);
                if (giana != null)
                    giana.Perform(schedule);
                else
                    schedule.Done(true);
            }

            #endregion

            #region Request

            request = Request.Get();

            if (request != null)
            {
                giana = Context.GetGianaAt(request.Port);
                if (giana != null)
                    giana.Perform(request);
                else
                    new Response(
                            (int)request.Id,
                            Response.Result.Error
                        ).Post();
            }

            #endregion

            Thread.Sleep(1000);

        }

    }

    #endregion

}
// =============================================================================
// CLASS Giana
// =============================================================================
class Giana
{

    #region Properties

    private SerialPort serialPort = null;

    public string Port
    {
        get { return serialPort.PortName; }
    }

    #endregion

    #region Constructors

    public Giana(SerialPort serialPort)
    {

        this.serialPort = serialPort;
        if (!serialPort.IsOpen)
            try
            {
                serialPort.Open();
            }
            catch (Exception exception)
            {
                Log.This(exception);
                Environment.Exit(1);
            }

    }

    #endregion

    #region Public Methods

    public void Perform(Schedule schedule)
    {

        bool value = ScheduleConditions(schedule.Conditions);

        Request request = new Request(
            null,
            schedule.Port, Action.Set, schedule.PinType,
            schedule.PinNumber,
            (value ? schedule.TrueValue : schedule.FalseValue)
            );

        Set(request);

        schedule.Done(false);

    }

    public void Perform(Request request)
    {

        Response response;

        if (request.Action == Action.Get)
            response = this.Get(request);
        else
            response = this.Set(request);

        if (response != null)
            response.Post();
        else
        {
            new Response(
                    request.Id,
                    Response.Result.Error
                ).Post();
        }

    }

    #endregion

    #region Private Methods

    private bool ScheduleConditions(ReadOnlyCollection<Schedule.Condition> conditions)
    {

        foreach (Schedule.Condition condition in conditions)
        {

            Request request = new Request(
                    null,
                    condition.Port, Action.Get, condition.PinType,
                    condition.PinNumber, 0
                );

            Response response = Get(request);

            switch (condition.Sign)
            {

                case '>':
                    if (int.Parse(response.Value) < condition.RawValue)
                        return false;
                    break;

                case '<':
                    if (int.Parse(response.Value) > condition.RawValue)
                        return false;
                    break;

                case '=':
                    if (int.Parse(response.Value) != condition.RawValue)
                        return false;
                    break;

            }

        }

        return true;

    }

    private Response Get(Request request)
    {

        string message = "<G";
        if (request.PinType == PinType.Digital) message += "D";
        else message += "A";
        if (request.PinNumber < 10) message += "0";
        message += request.PinNumber.ToString();
        message += ">";

        try
        {
            serialPort.Write(message);
        }
        catch (Exception exception)
        {
            Log.This(exception);
            Environment.Exit(1);
        }

        Thread.Sleep(500);

        string response = "";

        try
        {
            response = serialPort.ReadExisting().Trim();
        }
        catch (Exception exception)
        {
            Log.This(exception);
            Environment.Exit(1);
        }

        return new Response(
                request.Id,
                int.Parse(response.Substring
                    (4, request.PinType == PinType.Digital ? 1 : 4))
            );

    }

    private Response Set(Request request)
    {

        string message = "<SD";
        if (request.PinNumber < 10) message += "0";
        message += request.PinNumber.ToString();
        message += request.Value == 0 ? "0" : "1";
        message += ">";

        try
        {
            serialPort.Write(message);
        }
        catch (Exception exception)
        {
            Log.This(exception);
            Environment.Exit(1);
        }

        Thread.Sleep(500);

        string response = "";

        try
        {
            response = serialPort.ReadExisting().Trim();
        }
        catch (Exception exception)
        {
            Log.This(exception);
            Environment.Exit(1);
        }

        return new Response(
                request.Id,
                (response.Substring(6, 2) == "OK" ?
                    Response.Result.Ok : Response.Result.Error)
            );

    }

    #endregion

}
// =============================================================================
// CLASS Schedule
// =============================================================================
class Schedule
{

    #region Properties

    public int Id { get; private set; }

    public string Port { get; private set; }

    public PinType PinType { get; private set; }

    public byte PinNumber { get; private set; }

    public int TrueValue { get; private set; }

    public int FalseValue { get; private set; }

    public ReadOnlyCollection<Condition> Conditions { get; private set; }

    #endregion

    #region Public Methods

    public static Schedule Get()
    {

        string URI = Context.ServicesRootURI +
                                "schedule/";

        XmlDocument xmlDocument;

        try
        {
            xmlDocument = REST.This(URI, "GET");
        }
        catch (Exception exception)
        {
            Log.This(URI);
            Log.This(exception);
            return null;
        }

        if (xmlDocument.GetElementsByTagName("device").Count > 0)
        {

            Schedule schedule = new Schedule();

            XmlNodeList scheduleNode = xmlDocument.GetElementsByTagName("schedule");
            schedule.Id = int.Parse(scheduleNode.Item(0).Attributes["id"].Value);

            XmlNodeList deviceNode = xmlDocument.GetElementsByTagName("device");
            foreach (XmlNode xmlNode in deviceNode.Item(0).ChildNodes)
            {
                switch (xmlNode.Name)
                {
                    case "port":
                        schedule.Port = xmlNode.InnerText;
                        break;
                    case "pinType":
                        if (xmlNode.InnerText == "A")
                            schedule.PinType = PinType.Analogue;
                        else
                            schedule.PinType = PinType.Digital;
                        break;
                    case "pinNumber":
                        schedule.PinNumber = byte.Parse(xmlNode.InnerText);
                        break;
                    case "trueValue":
                        schedule.TrueValue = int.Parse(xmlNode.InnerText);
                        break;
                    case "falseValue":
                        schedule.FalseValue = int.Parse(xmlNode.InnerText);
                        break;
                }
            }

            List<Condition> listConditions = new List<Condition>();

            XmlNodeList conditionNode = xmlDocument.GetElementsByTagName("condition");
            foreach (XmlNode xmlNode in conditionNode)
            {

                string port = "";
                PinType pinType = PinType.Analogue;
                byte pinNumber = 0;
                char sign = ' ';
                int rawValue = 0;

                foreach (XmlNode childXmlNode in xmlNode.ChildNodes)
                {
                    switch (childXmlNode.Name)
                    {
                        case "port":
                            port = childXmlNode.InnerText;
                            break;
                        case "pinType":
                            if (childXmlNode.InnerText == "A")
                                pinType = PinType.Analogue;
                            else
                                pinType = PinType.Digital;
                            break;
                        case "pinNumber":
                            pinNumber = byte.Parse(childXmlNode.InnerText);
                            break;
                        case "sign":
                            sign = childXmlNode.InnerText.ToCharArray()[0];
                            break;
                        case "rawValue":
                            rawValue = int.Parse(childXmlNode.InnerText);
                            break;
                    }

                }

                listConditions.Add(
                    new Condition(port, pinType, pinNumber, sign, rawValue)
                    );

            }

            schedule.Conditions = listConditions.AsReadOnly();

            return schedule;

        }

        return null;

    }

    public void Done(bool error)
    {

        string URI = Context.ServicesRootURI +
                                "schedule/?scheduleId=" + Id +
                                "&error=" + (error ? "Y" : "N");

        try
        {
            REST.This(URI, "POST");
        }
        catch (Exception exception)
        {
            Log.This(URI);
            Log.This(exception);
        }

    }

    #endregion

    #region Nested Class Condition

    public class Condition
    {

        public string Port { get; private set; }

        public PinType PinType { get; private set; }

        public byte PinNumber { get; private set; }

        public char Sign { get; private set; }

        public int RawValue { get; private set; }

        public Condition(
                    string port,
                    PinType pinType,
                    byte pinNumber,
                    char sign,
                    int rawValue)
        {

            Port = port;
            PinType = pinType;
            PinNumber = pinNumber;
            Sign = sign;
            RawValue = rawValue;

        }

    }

    #endregion

}
// =============================================================================
// CLASS Request
// =============================================================================
class Request
{

    #region Properties

    public int? Id { get; set; }

    public string Port { get; set; }

    public Action Action { get; set; }

    public PinType PinType { get; set; }

    public byte PinNumber { get; set; }

    public int Value { get; set; }

    #endregion

    #region Constructor

    public Request(
            int? id,
            string port,
            Action action,
            PinType pinType,
            byte pinNumber,
            int value)
    {

        Id = id;
        Port = port;
        Action = action;
        PinType = pinType;
        PinNumber = pinNumber;
        Value = value;

    }

    #endregion

    #region Public Methods

    public static Request Get()
    {

        string URI = Context.ServicesRootURI +
                        "request/";

        XmlDocument xmlDocument;

        try
        {
            xmlDocument = REST.This(URI, "GET");
        }
        catch (Exception exception)
        {
            Log.This(URI);
            Log.This(exception);
            return null;
        }

        if (xmlDocument.GetElementsByTagName("action").Item(0) != null)
        {

            Request request = new Request(
                int.Parse(
                    xmlDocument.GetElementsByTagName("requestId").Item(0).InnerText
                ),
                xmlDocument.GetElementsByTagName("port").Item(0).InnerText,
                xmlDocument.GetElementsByTagName("action").Item(0).InnerText == "G" ?
                    Action.Get : Action.Set,
                xmlDocument.GetElementsByTagName("pinType").Item(0).InnerText == "A" ?
                    PinType.Analogue : PinType.Digital,
                byte.Parse(
                    xmlDocument.GetElementsByTagName("pinNumber").Item(0).InnerText
                    ),
                int.Parse(
                    xmlDocument.GetElementsByTagName("value").Item(0).InnerText
                    )
                );

            return request;

        }

        return null;

    }

    #endregion

}
// =============================================================================
// CLASS Response
// =============================================================================
class Response
{

    #region Properties

    public int? RequestId { get; private set; }

    public string Value { get; private set; }

    #endregion

    #region Constructor

    public Response(int? requestId, int value)
    {
        RequestId = requestId;
        Value = value.ToString();
    }

    public Response(int? requestId, Response.Result result)
    {
        RequestId = requestId;
        Value = (result == Result.Error ? Value = "ER" : "OK");
    }

    #endregion

    #region Public Methods

    public void Post()
    {

        if (RequestId == null) return;

        string URI = Context.ServicesRootURI +
                        "response/?requestId=" + RequestId +
                        "&value=" + Value;

        try
        {
            REST.This(URI, "POST");
        }
        catch (Exception exception)
        {
            Log.This(URI);
            Log.This(exception);
        }

    }

    #endregion

    #region Nested Enums

    public enum Result
    {
        Error,
        Ok
    }

    #endregion

}
// =============================================================================
// CLASS REST
// =============================================================================
static class REST
{

    #region Public Methods

    public static XmlDocument This(string URI, string method)
    {

        if (URI.Contains("?"))
            URI += "&SALT=" + Context.SALT;
        else
            URI += "?SALT=" + Context.SALT;

        HttpWebRequest httpWebRequest =
                (HttpWebRequest)(WebRequest.Create(URI));
        httpWebRequest.Method = method;
        HttpWebResponse httpWebResponse =
                (HttpWebResponse)(httpWebRequest.GetResponse());
        Stream stream = httpWebResponse.GetResponseStream();
        StreamReader streamReader = new StreamReader(stream, Encoding.UTF8);

        XmlDocument xmlDocument = new XmlDocument();
        string s = streamReader.ReadToEnd();
        xmlDocument.LoadXml(s);

        return xmlDocument;

    }

    #endregion

}
// =============================================================================
// CLASS Log
// =============================================================================
static class Log
{

    #region Public Methods

    public static void This(string message)
    {
        Console.WriteLine(DateTime.Now.ToString() + " | " + message);
    }

    public static void This(Exception exception)
    {
        Log.This("[EXCEPTION] " + exception.Message);
        if (exception.InnerException != null)
            Log.This(exception.InnerException);
    }

    #endregion

}
// =============================================================================
// ENUM Action
// =============================================================================
public enum Action
{
    Get,
    Set
}
// =============================================================================
// ENUM PinType
// =============================================================================
public enum PinType
{
    Analogue,
    Digital
}
// =============================================================================