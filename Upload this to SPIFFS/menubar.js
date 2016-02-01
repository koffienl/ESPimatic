var menu = '      <nav class="navbar navbar-default">' +
'         <div class="container-fluid">' +
'            <div class="navbar-header"><a class="navbar-brand" href="/">ESPimatic</a></div>' +
'            <div>' +
'               <ul class="nav navbar-nav">' +
'                  <li class="dropdown">' +
'                     <a class="dropdown-toggle" data-toggle="dropdown" href="#"><span class="glyphicon glyphicon-cog"></span> Configure<span class="caret"></span></a>' +
'                     <ul class="dropdown-menu">' +
'                        <li class="dropdown-submenu">' +
'                           <a tabindex="-1" href="#">Hardware</a>' +
'                           <ul class="dropdown-menu">' +
'                              <li><a tabindex="-1" href="/ds18b20.html">DS18B20<span class="glyphicon glyphicon-fire pull-right"></span></a></li>' +
'                              <li><a href="/ledmatrix.html">LED Matrix<span class="glyphicon glyphicon-equalizer pull-right"></span></a></li>' +
'                              <li><a href="/irled.html">IR LED<span class="glyphicon glyphicon-eye-open pull-right"></span></a></li>' +
'                              <li><a href="/relay.html">Relay<span class="glyphicon glyphicon-flash pull-right"></span></a></li>' +
'                              <li><a href="/dht.html">DHT<span class="glyphicon glyphicon-tint pull-right"></span></a></li>' +
'                           </ul>' +
'                        </li>' +
'                        <li class="dropdown-submenu">' +
'                           <a tabindex="-1" href="#">System</a>' +
'                           <ul class="dropdown-menu">' +
'							<li><a href="/wifi.html">WiFi<span class="glyphicon glyphicon-signal pull-right"></span></a></li>' +
'							<li><a href="/pimatic.html">Pimatic<span class="glyphicon glyphicon-transfer pull-right"></span></a></li>' +
'							<li><a href="/firmware.html">Firmware<span class="glyphicon glyphicon-upload pull-right"></span></a></li>' +
'							<li><a href="/#" data-toggle="modal" data-target="#modal-dialog" id="DoReset">Reset<span class="glyphicon glyphicon-erase pull-right"></span></a></li>' +
'							<li><a href="/filemanager.html">Filemanager<span class="glyphicon glyphicon-list-alt pull-right"></span></a></li>' +
'							<li><a href="/esp.html">System<span class="glyphicon glyphicon-tasks pull-right"></span></a></li>' +
'                           </ul>' +
'                        </li>' +
'                     </ul>' +
'					 <li class=""><a href="#" id="refresh"><span class="glyphicon glyphicon-repeat"></span> Refresh</a></li>' +
'					 <li class=""><a href="#" data-toggle="modal" data-target="#modal-dialog" id="DoReboot"><span class="glyphicon glyphicon-off"></span> Reboot</a></li>' +
'					 <li><a href="/login_ajax?action=logoff" style="display: none;" id="logoff"><span class="glyphicon glyphicon-lock"></span> Logoff</a></li>' +
'					 <li><a href="https://github.com/koffienl/ESPimatic/wiki" target="_blank"><span class="glyphicon glyphicon-question-sign"></span> Help</a></li>' +
'					 <li><a href="https://github.com/koffienl/ESPimatic/issues" target="_blank"><span class="glyphicon glyphicon-question-sign"></span> Support</a></li>' +
'                  </li>' +
'               </ul>' +
'            </div>' +
'         </div>' +
'      </nav>';
document.write(menu);