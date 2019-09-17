////////////////////
//index.htm
////////////////////////

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html charset="UTF-8">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  
  <!-- Chart.js -->
  <script src = "https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.8.0/Chart.min.js"></script>  
  
  <style>
    td {border: 1px solid black; text-align: right;}
    th {border: 1px solid black; background-color:#008080;color:white;text-align: left;} 
    ul {list-style-type: none;margin: 0;padding: 0;overflow: hidden;background-color: #008080;}
    li {float: left;}
    li a {display: block;color: white;text-align: center;padding: 10px;text-decoration: none;}
    li a:hover {background-color: #2F4F4F;}
    .border {border-style:solid;border-width: thin;border-color:#000000;}
  </style>
</head>
<body>
  <p>
    <span class="border" id="online">&nbsp;</span><strong> ESP8266 Monitor Baterias Web Server </strong>[<span id="datetime"></span>]
  </p>
  <ul>
    <li><a href="#tab_1" onclick="setTab_1()">Medidas</a></li>
    <li><a href="#tab_2" onclick="setTab_2()">Balance</a></li>
    <li><a onclick="setTab_3()">Graficas</a></li> <!-- href="#tab_3" -->
  </ul>
  <br/> 
  <div id="tab_1"> 
    <table>
      <thead>
        <tr>
          <th>DS_temp</th>
          <th>DHT_temp</th>
          <th>DHT_hum</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><span id="ds_temp"></span> &deg;C</td>
          <td><span id="dht_temp"></span> &deg;C</td>
          <td><span id="dht_hum"></span> %</td>
        </tr>
      </tbody>
    </table>
    <br/>
    <table>
      <thead>
        <tr>
          <th>V0</th>
          <th>A1</th>
          <th>A2</th>
          <th>A3</th>
          <th>A4</th>
          <th>A5</th>
          <th>V6</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><span id="V0"></span> V</td>
          <td><span id="A1"></span> A</td>
          <td><span id="A2"></span> A</td>
          <td><span id="A3"></span> A</td>
          <td><span id="A4"></span> A</td>
          <td><span id="A5"></span> A</td>
          <td><span id="V6"></span> V</td>
        </tr>
      </tbody>  
    </table>
    <br/>
    <p>
      <button id="brelay" type="button" onclick="brelay()">Rele</button>
      <button id="bLD" type="button" onclick="bLD()">EP DC Load</button>
      <button id="bBlynk" type="button" onclick="bBlynk()">Blynk</button>
      <span class="border" id="blynk_st">&nbsp;</span>
      <button id="bModbus" type="button" onclick="bModbus()">Modbus</button>
      <span class="border" id="modbus_st">&nbsp;</span>
      <span id="modbus_code"></span>
    </p>
    <div id="pmodbus">
      <table>
        <tbody>
          <tr>
            <th>PV</th>
            <td><span id="EP_PV_volt"></span> V [<span id="EP_pv_min_volt_today"></span> / <span id="EP_pv_max_volt_today"></span>]</td>
            <td><span id="EP_PV_cur"></span> A</td>
            <td><span id="EP_PV_pw"></span> W</td>
          </tr>
          <tr>
            <th>BAT</th>
            <td><span id="EP_BAT_volt"></span> V [<span id="EP_bat_min_volt_today"></span> / <span id="EP_bat_max_volt_today"></span>]</td>
            <td><span id="EP_BAT_cur"></span> A</td>
            <td><span id="EP_BAT_pw"></span> W</td>
          </tr>
          <tr>
            <th>LD</th>
            <td><span id="EP_LD_volt"></span> V</td>
            <td><span id="EP_LD_cur"></span> A</td>
            <td><span id="EP_LD_pw"></span> W</td>
          </tr>
        </tbody>
      </table>
      <br/>
      <table>
        <thead>
          <tr>
            <th>BAT_SOC</th>
            <th>T_bat</th>
            <th>T_inside</th>
            <th>batST</th>
            <th>sysST</th>
            <th>ldST</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td><span id="EP_BAT_SOC"></span><span> %</span></td>
            <td><span id="EP_T_bat"></span> &deg;C</td>
            <td><span id="EP_T_inside"></span> &deg;C</td>
            <td><span id="EP_batstatus"></span></td>
            <td><span id="EP_sysstatus"></span></td>
            <td><span id="EP_ldstatus"></span></td>
          </tr>
        </tbody>  
      </table>
      <br/>
      <table>
        <tbody>
          <tr>
            <td></td>
            <th>T</th>
            <th>M</th>
            <th>Y</th>
            <th>Tot</th>
          </tr>
          <tr>
            <th>Gen (kwh)</th>
            <td><span id="EP_genEnT"></span></td>
            <td><span id="EP_genEnM"></span></td>
            <td><span id="EP_genEnY"></span></td>
            <td><span id="EP_genEnTot"></span></td>
          </tr>
          <tr>
            <th>Cons (kwh)</th>
            <td><span id="EP_conEnT"></span></td>
            <td><span id="EP_conEnM"></span></td>
            <td><span id="EP_conEnY"></span></td>
            <td><span id="EP_conEnTot"></span></td>
          </tr>
        </tbody>
      </table>
    </div>
    <p>
      <button type="button" id="bReboot" onclick="bReboot()">Reboot</button>
    </p> 
  </div>
  <div id="tab_2">
    <br/>
    <table>
      <tbody>
        <tr>
          <td>Load Volt: <span id="LV"></span> (V)&nbsp;</td>
          <th>Power (W)</th>
          <th>Current [Res.] (A)</th>
          <th>Energy (Kwh)</th>
        <tr>
          <th>PV</th>
          <td><span id="P_PV"></span></td>
          <td><span id="C_PV"></span></td>
          <td><span id="E_PV"></span></td>
        </tr>
        <tr>
          <th>->DC LOAD</th>
          <td><span id="P_DLD"></span></td>
          <td><span id="C_DLD"></span></td>
          <td><span id="E_DLD"></span></td>
        </tr>
        <tr>
          <th>->PV - DC LOAD</th>
          <td><span id="P_DGEN"></span></td>
          <td><span id="C_DGEN"></span></td>
          <td><span id="E_DGEN"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;->HOME CONS.</th>
          <td><span id="P_H"></span></td>
          <td><span id="C_H"></span></td>
          <td><span id="E_H"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;->BANK CHARGE</th>
          <td><span id="P_BANK"></span></td>
          <td><span id="C_BANK"></span></td>
          <td><span id="E_BANK"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;->BAT 1</th>
          <td><span id="P_B1"></span></td>
          <td><span id="C_B1"></span> [<span id="C_R_B1"></span>] </td>
          <td><span id="E_B1"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;->BAT 2</th>
          <td><span id="P_B2"></span></td>
          <td><span id="C_B2"></span> [<span id="C_R_B2"></span>] </td>
          <td><span id="E_B2"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;->BAT 3</th>
          <td><span id="P_B3"></span></td>
          <td><span id="C_B3"></span> [<span id="C_R_B3"></span>] </td>
          <td><span id="E_B3"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;->BAT 4</th>
          <td><span id="P_B4"></span></td>
          <td><span id="C_B4"></span> [<span id="C_R_B4"></span>] </td>
          <td><span id="E_B4"></span></td>
        </tr>
        <tr>
          <th>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;->BAT 5</th>
          <td><span id="P_B5"></span></td>
          <td><span id="C_B5"></span> [<span id="C_R_B5"></span>] </td>
          <td><span id="E_B5"></span></td>
        </tr>
      </tbody>
    </table>
    <p>
      <button type="button" id="bReset" onclick="bReset()">Reset</button>
    </p> 
  </div> 
  <div id="tab_3">
    <div class="chart-container" style="position: relative; height:60vh; width:85vw">
      <canvas id="Chart_1"></canvas>
    </div>
    <br/>
    <div class="chart-container" style="position: relative; height:60vh; width:85vw">
      <canvas id="Chart_2"></canvas>
    </div>
    <p>
      <button type="button" id="bRefresh" onclick="bRefresh()">Refresh</button>
      <button type="button" id="bView" onclick="bView()">View</button>
    </p> 
  </div>
  <br/>
  <span id="info"></span>
  <br/>
  <span id="info2"></span>
</body>

<script>

var ctx_1 =  document.getElementById("Chart_1").getContext('2d'), chart_1;//Grafica 1 
var ctx_2 =  document.getElementById("Chart_2").getContext('2d'), chart_2;//Grafica 2

window.onload = function() {
  setTab_1();
  chart_1 = createChart(ctx_1,"Medidas actuales");
  chart_2 = createChart(ctx_2,"Medidas periodo");
  chart_2.options.elements.point.radius = 0 ;//No Dots
  getData();
  setInterval(getData, 3000);
  getEPData();
  setInterval(getEPData, 5000 ) ;//MB_TIME
  getpData();
  setInterval(getpData, 3000);
  //setTimeout(function(){getHistData(chart_2);}, 5000);
};

function setTab_1() {
  document.getElementById("tab_1").style.display = "block";
  document.getElementById("tab_2").style.display = "none";
  document.getElementById("tab_3").style.display = "none";
}
function setTab_2() {
  document.getElementById("tab_1").style.display = "none";
  document.getElementById("tab_2").style.display = "block";
  document.getElementById("tab_3").style.display = "none";
}
function setTab_3() {
  document.getElementById("tab_1").style.display = "none";
  document.getElementById("tab_2").style.display = "none";
  document.getElementById("tab_3").style.display = "block";
}

//Graphs visit: https://www.chartjs.org

chartColors = {
  maroon: '#800000',
  brown: '#9A6324',
  olive: '#808000',
  teal: '#469990',
  navy: '#000075',
  black: '#000000',
  red: '#e6194b',
  orange: '#f58231',
  yellow: '#ffe119',
  lime: '#bfef45',
  green: '#3cb44b',
  cyan: '#42d4f4',
  blue: '#4363db',
  purple: '#911eb4',
  magenta: '#f032e6',
  grey: '#a9a9a9',
  pink: '#fabebe',
  apricot: '#ffd8b1',
  beige: '#fffac8',
  mint: '#aaffc3',
  lavender: '#e6beff',
  white: '#ffffff'
};


function createChart(ctx, tit) // canvas por referencia, title
{
   var ch = new Chart(ctx, {
        type: 'line',
        data: {
            //labels: timeStamp,  //Bottom Labeling
            datasets: [{
                label: "Te [\u00B0C]",
                fill: false,  
                backgroundColor: chartColors.orange, //Dot marker color
                borderColor: chartColors.orange, //Graph Line Color
                yAxisID: 'y-axis-C',
            },
            {
                label: "Hu [%]",
                fill: false,  
                backgroundColor: chartColors.blue,
                borderColor: chartColors.blue,
                yAxisID: 'y-axis-%',
            },
            {
                label: "PV",
                fill: false,  
                backgroundColor: chartColors.navy,
                borderColor: chartColors.navy,
                yAxisID: 'y-axis-W',
            },
            {
                label: "DC_LD",
                fill: false,  
                backgroundColor: chartColors.magenta,
                borderColor: chartColors.magenta,
                yAxisID: 'y-axis-W',
            },
            {
                label: "H_LD",
                fill: false,  
                backgroundColor: chartColors.teal,
                borderColor: chartColors.teal,
                yAxisID: 'y-axis-W',
            },
            {
                label: "BANK",
                fill: false,  
                backgroundColor: chartColors.red,
                borderColor: chartColors.red,
                yAxisID: 'y-axis-W',
            },
            {
                label: "B1",
                fill: false,  
                backgroundColor: chartColors.maroon,
                borderColor: chartColors.maroon,
                yAxisID: 'y-axis-W',
                hidden: true,
            },
            {
                label: "B2",
                fill: false,  
                backgroundColor: chartColors.brown,
                borderColor: chartColors.brown,
                yAxisID: 'y-axis-W',
                hidden: true,
            },
            {
                label: "B3",
                fill: false,  
                backgroundColor: chartColors.olive,
                borderColor: chartColors.olive,
                yAxisID: 'y-axis-W',
                hidden: true,
            },
            {
                label: "B4",
                fill: false,  
                backgroundColor: chartColors.lime,
                borderColor: chartColors.lime,
                yAxisID: 'y-axis-W',
                hidden: true,
            },
            {
                label: "B5",
                fill: false,  
                backgroundColor: chartColors.green,
                borderColor: chartColors.green,
                yAxisID: 'y-axis-W',
                hidden: true,
            },
            {
                label: "V [V]",
                fill: false,  
                backgroundColor: chartColors.black,
                borderColor: chartColors.black,
                yAxisID: 'y-axis-V',
            }]
        },
        options: {
            title: {display: true, text: tit},
            responsive: true,
            maintainAspectRatio: false,
            elements: {
              line: {tension: 0.5 } //Smoothening (Curved) of data lines
            },
            scales: {
              yAxes: [
                {type: 'linear', display: true, position:  'right', id: 'y-axis-W', scaleLabel: {display: true,labelString: 'Watts'}, ticks: {beginAtZero:false } },
                {type: 'linear', display: true, position:  'right', id: 'y-axis-V', scaleLabel: {display: true,labelString: 'Volts'}, ticks: {beginAtZero:false }, gridLines: {drawOnChartArea: false} },
                {type: 'linear', display: true, position:  'left', id: 'y-axis-C', scaleLabel: {display: true,labelString: 'Temperature'}, ticks: {beginAtZero:false }, gridLines: {drawOnChartArea: false} },
                {type: 'linear', display: true, position:  'left', id: 'y-axis-%', scaleLabel: {display: true,labelString: '%'}, ticks: {beginAtZero:false }, gridLines: {drawOnChartArea: false} },          
              ]
            },
            //animation: {duration: 0 }, // general animation time
            //hover: {animationDuration: 0 }, // duration of animations when hovering an item
            //responsiveAnimationDuration: 0 // animation duration after a resize
        }
   });
   return ch;  
}


function getData( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      document.getElementById("ds_temp").innerHTML = data.ds_temp;
      document.getElementById("dht_temp").innerHTML = data.dht_temp;
      document.getElementById("dht_hum").innerHTML = data.dht_hum;
      document.getElementById("V0").innerHTML = data.V0;
      document.getElementById("A1").innerHTML = data.A1;
      document.getElementById("A2").innerHTML = data.A2;
      document.getElementById("A3").innerHTML = data.A3;
      document.getElementById("A4").innerHTML = data.A4;
      document.getElementById("A5").innerHTML = data.A5;
      document.getElementById("V6").innerHTML = data.V6;
      document.getElementById("info").innerHTML = data.info;
      document.getElementById("info2").innerHTML = data.info2;
      document.getElementById("datetime").innerHTML = data.datetime;
      
      if (data.relay==1) {document.getElementById("brelay").style.backgroundColor="greenyellow";} else {document.getElementById("brelay").style.backgroundColor="LightGrey";}

      document.getElementById("online").style.backgroundColor="greenyellow";

      document.getElementById("modbus_st").style.backgroundColor="LightGrey";
      document.getElementById("modbus_code").innerHTML = "";
      if (data.modbus_st == -1) { //Modbus Disabled
          document.getElementById("bModbus").style.backgroundColor="LightGrey";
          document.getElementById("pmodbus").style.display = "none";
      } else {
          document.getElementById("bModbus").style.backgroundColor="greenyellow";
          document.getElementById("pmodbus").style.display = "block";
          if (data.modbus_st==0) {//Modbus comms OK
            document.getElementById("modbus_st").style.backgroundColor="greenyellow";
          } else {
            document.getElementById("modbus_code").innerHTML = " 0x"+ data.modbus_st.toString(16);
          } 
      }
      
      document.getElementById("blynk_st").style.backgroundColor="LightGrey";
      if (data.blynk_st == -1) { //Blynk Disabled
          document.getElementById("bBlynk").style.backgroundColor="LightGrey";
      } else {
          document.getElementById("bBlynk").style.backgroundColor="greenyellow";
          if (data.blynk_st==1) document.getElementById("blynk_st").style.backgroundColor="greenyellow";//Blynk comms OK
      }

    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.timeout = 5000 //2500;
  xhttp.ontimeout = function (e) {
    document.getElementById("online").style.backgroundColor="LightGrey";
  };
  xhttp.send();
}


function brelay( ) { //relay OFF ON
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (this.responseText==1) {document.getElementById("brelay").style.backgroundColor="greenyellow";} else {document.getElementById("brelay").style.backgroundColor="LightGrey";}
    }
  };
  xhttp.open("GET", "/relay", true);
  xhttp.send();
};

function bBlynk( ) { //BLYNK COMMS OFF ON
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (this.responseText== 0) {
          document.getElementById("bBlynk").style.backgroundColor="LightGrey";
        } else {
          document.getElementById("bBlynk").style.backgroundColor="greenyellow";
        }
    }
  };
  xhttp.open("GET", "/blynk_en", true);
  xhttp.send();
};

function bModbus( ) { //MODBUS COMMS OFF ON
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (this.responseText == 0) {
          document.getElementById("bModbus").style.backgroundColor="LightGrey";
          document.getElementById("pmodbus").style.display = "none";
        } else {
          document.getElementById("bModbus").style.backgroundColor="greenyellow";
          document.getElementById("pmodbus").style.display = "block";
        }
    }
  };
  xhttp.open("GET", "/modbus_en", true);
  xhttp.send();
};

function getEPData( ) {
  if (document.getElementById("pmodbus").style.display === "none") return;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      document.getElementById("EP_PV_volt").innerHTML = data.PV_volt;
      document.getElementById("EP_PV_cur").innerHTML = data.PV_cur;
      document.getElementById("EP_PV_pw").innerHTML = data.PV_pw;
      document.getElementById("EP_BAT_volt").innerHTML = data.BAT_volt;
      document.getElementById("EP_BAT_cur").innerHTML = data.BAT_cur;
      document.getElementById("EP_BAT_pw").innerHTML = data.BAT_pw;
      document.getElementById("EP_BAT_SOC").innerHTML = data.BAT_SOC;
      document.getElementById("EP_LD_volt").innerHTML = data.LD_volt;
      document.getElementById("EP_LD_cur").innerHTML = data.LD_cur;
      document.getElementById("EP_LD_pw").innerHTML = data.LD_pw;
      document.getElementById("EP_T_bat").innerHTML = data.T_bat;
      document.getElementById("EP_T_inside").innerHTML = data.T_inside;
      
      if (data.LD_ST==1) {document.getElementById("bLD").style.backgroundColor="greenyellow";} else {document.getElementById("bLD").style.backgroundColor="LightGrey";}

      document.getElementById("EP_batstatus").innerHTML = data.batstatus;
      document.getElementById("EP_sysstatus").innerHTML = data.sysstatus;
      document.getElementById("EP_ldstatus").innerHTML = data.ldstatus;
      document.getElementById("EP_pv_max_volt_today").innerHTML = data.pv_max_volt_today;
      document.getElementById("EP_pv_min_volt_today").innerHTML = data.pv_min_volt_today;
      document.getElementById("EP_bat_max_volt_today").innerHTML = data.bat_max_volt_today;
      document.getElementById("EP_bat_min_volt_today").innerHTML = data.bat_min_volt_today;
      document.getElementById("EP_conEnT").innerHTML = data.conEnT;
      document.getElementById("EP_conEnM").innerHTML = data.conEnM;
      document.getElementById("EP_conEnY").innerHTML = data.conEnY;
      document.getElementById("EP_conEnTot").innerHTML = data.conEnTot;
      document.getElementById("EP_genEnT").innerHTML = data.genEnT;
      document.getElementById("EP_genEnM").innerHTML = data.genEnM;
      document.getElementById("EP_genEnY").innerHTML = data.genEnY;
      document.getElementById("EP_genEnTot").innerHTML = data.genEnTot;
    
    } 
  };
  xhttp.open("GET", "/EP_data", true);
  xhttp.send();
}

function bLD( ) { //EP LOAD OFF ON
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (this.responseText==1) {document.getElementById("bLD").style.backgroundColor="greenyellow";} else {document.getElementById("bLD").style.backgroundColor="LightGrey";}
    }
  };
  xhttp.open("GET", "/EP_LD", true);
  xhttp.send();
};

function bReboot( ) { //REBOOT ESP
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/reboot", false);
  xhttp.send();
};

function adddataChart(ch,t,v, mx) { //chart, time, array current values, limit shift
  var datasets = ch.data.datasets;
  var labels = ch.data.labels;
  var n = labels.length;
  if (n > mx) {
    labels.shift();
    for (var i = 0; i < datasets.length; i++) datasets[i].data.shift();  
  }
  //Push data in chart array
  labels.push(t);
  for (var i = 0; i < datasets.length; i++) datasets[i].data.push(v[i]); 
  ch.update();
}

function getpData( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      var v = parseFloat (data.V);
      var bpc = parseFloat (data.P_BANK)/v/5.0;//Battery proportional current
      
      document.getElementById("LV").innerHTML = data.V;
      
      document.getElementById("P_PV").innerHTML = data.P_PV;
      document.getElementById("C_PV").innerHTML = (parseFloat(data.P_PV)/v).toFixed(2);
      document.getElementById("E_PV").innerHTML = data.E_PV;  
      
      document.getElementById("P_DLD").innerHTML = data.P_DLD;
      document.getElementById("C_DLD").innerHTML = (parseFloat(data.P_DLD)/v).toFixed(2);
      document.getElementById("E_DLD").innerHTML = data.E_DLD;
      
      document.getElementById("P_DGEN").innerHTML = data.P_DGEN;
      document.getElementById("C_DGEN").innerHTML = (parseFloat(data.P_DGEN)/v).toFixed(2);
      document.getElementById("E_DGEN").innerHTML = data.E_DGEN;
      
      document.getElementById("P_H").innerHTML = data.P_H;
      document.getElementById("C_H").innerHTML = (parseFloat(data.P_H)/v).toFixed(2);
      document.getElementById("E_H").innerHTML = data.E_H; 
      
      document.getElementById("P_BANK").innerHTML = data.P_BANK;
      document.getElementById("C_BANK").innerHTML = (parseFloat(data.P_BANK)/v).toFixed(2);
      document.getElementById("E_BANK").innerHTML = data.E_BANK;
      
      document.getElementById("P_B1").innerHTML = data.P_B1;
      document.getElementById("C_B1").innerHTML = (parseFloat(data.P_B1)/v).toFixed(2);
      document.getElementById("C_R_B1").innerHTML = (parseFloat(data.P_B1)/v - bpc).toFixed(2);
      document.getElementById("E_B1").innerHTML = data.E_B1;  
      
      document.getElementById("P_B2").innerHTML = data.P_B2;
      document.getElementById("C_B2").innerHTML = (parseFloat(data.P_B2)/v).toFixed(2);
      document.getElementById("C_R_B2").innerHTML = (parseFloat(data.P_B2)/v - bpc).toFixed(2);
      document.getElementById("E_B2").innerHTML = data.E_B2;  
      
      document.getElementById("P_B3").innerHTML = data.P_B3;
      document.getElementById("C_B3").innerHTML = (parseFloat(data.P_B3)/v).toFixed(2);
      document.getElementById("C_R_B3").innerHTML = (parseFloat(data.P_B3)/v - bpc).toFixed(2);
      document.getElementById("E_B3").innerHTML = data.E_B3;  
      
      document.getElementById("P_B4").innerHTML = data.P_B4;
      document.getElementById("C_B4").innerHTML = (parseFloat(data.P_B4)/v).toFixed(2);
      document.getElementById("C_R_B4").innerHTML = (parseFloat(data.P_B4)/v - bpc).toFixed(2);
      document.getElementById("E_B4").innerHTML = data.E_B4;  
      
      document.getElementById("P_B5").innerHTML = data.P_B5;
      document.getElementById("C_B5").innerHTML = (parseFloat(data.P_B5)/v).toFixed(2);
      document.getElementById("C_R_B5").innerHTML = (parseFloat(data.P_B5)/v - bpc).toFixed(2);
      document.getElementById("E_B5").innerHTML = data.E_B5;    

      //Push the data in chart array
      var t = new Date().toLocaleTimeString(); 
      adddataChart(chart_1, t,[data.temp, data.hum, data.P_PV, data.P_DLD, data.P_H, data.P_BANK, data.P_B1, data.P_B2, data.P_B3, data.P_B4, data.P_B5, data.V],40);
      
    }
  };
  xhttp.open("GET", "/pdata", true);
  xhttp.timeout = 2500
  xhttp.ontimeout = function (e) {
  };
  xhttp.send();
}

function bReset( ) { //Reset Energy
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/pdata_reset", false);
  xhttp.send();
};

function bRefresh( ) { //Refresh Chart
  getHistData(chart_2);
};

function getHistData(ch,ix) { //Chart by Reference anda data index
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      
      if (ix === undefined) {
        ch.data.labels = JSON.parse(this.responseText);
        getHistData(ch,0);
      } else {
        ch.data.datasets[ix++].data = JSON.parse(this.responseText);
        ch.update();
        if (ix<12) getHistData(ch,ix);
      }
      
    }    
  };
  xhttp.open("GET", (ix === undefined) ? "/hist_data" : "/hist_data?ix="+ix, true);
  xhttp.timeout = 10000
  xhttp.ontimeout = function (e) {
    //alert("Timeout");
  };
  xhttp.send();
}

var viewST=0;
function bView( ) { //Change Fields View
  viewST++;if (viewST >4 ) viewST=0;
  changeView(chart_1,viewST);
  changeView(chart_2,viewST);
};

function changeView(ch,v) { //Change chart views
  ch.data.datasets.forEach( function(e,i) {ch.getDatasetMeta(i).hidden=true; });
  ch.options.scales.yAxes.forEach( function(e,i) {e.display=false; e.gridLines.drawOnChartArea=false; });
  if (v == 1 || v == 0) { //Temp and Humidity
    ch.getDatasetMeta(0).hidden=false;
    ch.getDatasetMeta(1).hidden=false;
    ch.options.scales.yAxes[2].display=true;
    ch.options.scales.yAxes[3].display=true;
    if (v) ch.options.scales.yAxes[2].gridLines.drawOnChartArea=true;

  };
  if (v == 2 || v == 0) { //Watts Panel, Load, Home, Bank
    ch.getDatasetMeta(2).hidden=false;
    ch.getDatasetMeta(3).hidden=false;
    ch.getDatasetMeta(4).hidden=false;
    ch.getDatasetMeta(5).hidden=false;
    ch.options.scales.yAxes[0].display=true;
    ch.options.scales.yAxes[0].gridLines.drawOnChartArea=true;
  };
  if (v == 3) { //Watts BANK and Batteries
    ch.getDatasetMeta(5).hidden=false;
    ch.getDatasetMeta(6).hidden=false;
    ch.getDatasetMeta(7).hidden=false;
    ch.getDatasetMeta(8).hidden=false;
    ch.getDatasetMeta(9).hidden=false;
    ch.getDatasetMeta(10).hidden=false;
    ch.options.scales.yAxes[0].display=true;
    ch.options.scales.yAxes[0].gridLines.drawOnChartArea=true;
  };
  if (v == 4 || v == 0) { //Voltage
    ch.getDatasetMeta(11).hidden=false;
    ch.options.scales.yAxes[1].display=true;
    if (v) ch.options.scales.yAxes[1].gridLines.drawOnChartArea=true;
  }
  ch.update();
};



</script> 
</html>
)rawliteral";
