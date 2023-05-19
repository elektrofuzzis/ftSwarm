---
title: FAQ
layout: category
lang: de
classes: wide
---

### Firmware

<details>
<summary>Ich habe auf meinem Controller ein eigenes Programm laufen und möchte nun die Standard-Firmware einspielen.</summary>
<p>Spielen Sie einfach das letzte <href="../update">firmware update</a> ein.</p>
</details>

<hr>

<details>
<summary>Nach einem Update funktioniert mein Controller nicht mehr.</summary>
<p>Schließen Sie ein Terminalprogramm wie putty an. Starten Sie den Controller über die Reset-Taste. Der Controller startet ein Initialsetup.</p>
<p>Geben Sie die Daten vom Typenschild Ihres Controllers ein. Anschließend bootet der Controller neu.</p>
<p>In der NVS Partition ihres Controllers sind einige Einstellungen wie die Seriennummer hinterlegt. Wird diese Partition gelöscht oder ist für die Firmware nicht lesbar, so fragt die Firmware die Daten einmalig ab.</p>
</details>

<hr>

<details>
<summary>Mein Controller lässt sich nicht in den Schwarm einbinden.</summary>
<p>Kontrollieren Sie zunächst, dass der Controller und der Schwarm mit dem selben wifi-Netzwerk verbunden sind:</p>
<ul>
<li>Ist WLAN eingeschaltet?</li>
<li>Stimmen SSID und Passwort?</li>
<li>Können die Statusseiten der Controller vom PC aufgerufen werden?</li>
</ul>
<p>Verwenden Sie beim ftSwarmRS die RS485-Schnittstelle?</p>
<ul>
<li>Steht die Schwarmkommunikation bei allen Controllern auf RS485?</li>
<li>Ist die Verkabelung zwischen den Controllern in Ordnung? Sind alle Kabel angeschlossen? Haben Sie A/B vertauscht? Haben sie eine Schleife gebaut?</li>
<li>Haben Sie am ersten und letzten Controller den Jumper für den Abschlusswiderstand gesetzt?</li>
</ul>
<p>Bei einigen älteren Firmwareversionen hatten manche User Probleme mit der Arduino IDE 2.0.</p>
</details>

<hr>

<details>
<summary>Mein Controller erzeugt ein eigenes WLAN, obwohl er auf Client-Modus eingestellt wurde.</summary>
<p>Wenn Sie einen SSID-Scan am PC laufen lassen, sehen Sie für jeden Controller eine eigene SSID, z.B. ftSwarm4711.<br>
Ignorieren Sie diese SSID. Es ist ein Bug von espressif.</p>
</details>

### Programmierung

<details>
<summary>Mein Programm crashed mit einer Watchdog-Fehlermeldung</summary>

<p>Die ESP32-Prozessoren verwenden einen Watchdog, um (fast-)Endlosschleifen zu erkennen und abzubrechen.</p>

<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>while (switch->isReleased());
motor->setSpeed(200);</code></pre></div></div>

<p>blockiert einen Core bis der Taster geschlossen wird, der Watchdog erkennt dies und bricht das Programm ab. Es ist besser in der <strong>loop</strong>-Funktion</p>

<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>if (switch->ToggleUp()) motor->setSpeed(200);</code></pre></div></div>

<p>zu verwenden. Da die Loop-Funktion immer wieder neu gestartet wird, wird auch das if-Statement regelmäßig ausgeführt. Damit wird der Motor ebenfalls eingeschaltet, wenn der Taster geschlossen wird. Der Watchdog bricht nicht ab, da die <strong>loop</strong>-Funktion komplett durchlaufen wurde. Die <strong>delay</strong>-Funktion sorgt ebenfalls dafür, dass der Watchdog das Programm nicht unterbricht.</p>

</details>

<hr>

<details>
<summary>Die Motoren am Remote-Controller arbeiten nicht korrekt.</summary>
<p>Die Statusseite zegt den Motor korrekt an, aber der Motor läuft nicht.<br>Dies ist ein bekanntes Problem, das mit Firmware 0.4.1 gefixt wurde.</p>
</details>

<hr>

<details>
<summary>Die Eingänge funktionieren nicht</summary>
<p>Überprüfen Sie die 9V-Stromversorgung. Um die Eingänge zu nutzen, reicht die USB-Stromversorgung nicht aus.</p>
</details>

<hr>

<details>
<summary>Der Kelda-Controller "hängt" nach dem Flashen oder Reboot des Controllers.</summary>
<p>Sie verwenden den Kelda-Controller im AP-Modus und alle anderen Controller im Schwarm verbinden sich auf die Kelda über wifi.</p>
<p>Prüfen Sie, ob die Kelda "waiting on hardware" meldet. (Beim ftSwarmControl sehen Sie dies im Display, beim ftSwarm sind beide LEDs blau.)</p>
<p>Beim Flashen bzw. beim Restart des Controllers, wird der WLAN-Stack neu initialisiert. Wird der Controller neu gestartet, der den AP stellt, so verlieren alle damit verbundenen Controller die Verbindung und können nicht mehr kommunizieren. Die Kelda versucht auf Remote-Hardware zuzugreifen und kann sie nicht mehr ansprechen. Alle Controller im Swarm müssen neu gestartet werden.</p>
<p>Es ist jedoch sinnlos immer alle Controller nach dem Flashen der Kelda zu resetten. Verwenden Sie ein extern bereitgestelltes WLAN oder nutzen Sie einen anderen Controller als Access Point.</p>
</details>

<hr>

<details>
<summary>Ungewöhnliches toggle-Verhalten</summary>
<p>Schauen Sie sich folgenden Code an (er funktioniert mit jedem Eingangstyp):</p>
<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>void loop() {

  switch ( local_S1->getToggle() ) {
    case FTSWARM_TOGGLEDOWN:  Serial.println("S1 released."); break;
    case FTSWARM_TOGGLEUP:    Serial.println("S1 pressed."); break;
  }
  
  delay(250);

}</code></pre></div></div>
<p>Wenn das Programm läuft, erhalten Sie mit dem Drücken und Loslassen der Taster <strong>S1 pressed.</strong> und <strong>S1 released.</strong> Nachrichten. In manchen Fällen, kommt es aber zu zwei aufeinander folgenden <strong>S1 pressed.</strong> Meldungen, die zugehörige <strong>S1 released.</strong> Nachricht fehlt.</p>
<p>Der Schwarm prüft aller 25ms die Eingänge. Beim Auslesen von S1 vergleicht die Firmwae den neuen Stand (z.B. gedrückt) mit dem vorherigen Zustand (z.B. nicht gerdückt). Dadurch wird der Toggle-Zustand auf <strong>FTSWARM_TOGGLEUP</strong> gesetzt.</p>
<p>Bleibt der Taster gedrückt, so prüft die <strong>loop</strong>-Funktion spätestens nach 250ms den Zustand und gibt auf der seriellen Konsole <strong>S1 pressed</strong> aus. <strong>local_S1->getToggle()</strong> setzt dabei den Status des Eingangs auf <strong>FTSWARM_NOTOGGLE</strong>. Beim nächsten Durchlauf der loop ist der Zustand <strong>local_S1->getToggle()</strong> und es wird nichts ausgegeben.</p>
<p>Nachdem der Taster losgelassen wurde, ändert sich der Zustand zu <strong>FTSWARM_TOGGLEDOWN</strong>. Auf der Konsole wird <strong>S1 released</strong> ausgegeben.</p>
<p>Wird der Taster, nachdem er gedrückt wurde (Ausgabe <strong>S1 pressed.</strong>), innerhalb der 250ms losgelassen und erneut gedrückt, so ändert sich der Zustand des Tasters während dem <strong>delay</strong> zweimal. Am Ende der 250ms steht der Taster auf <strong>FTSWARM_TOGGLEUP</strong>.</p>
<p>Damit wird zum zweitem Mal in Folge <strong>S1 pressed.</strong> ausgegeben.</p>
</details>