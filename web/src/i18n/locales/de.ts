const de = {
  translation: {
    head: {
      desktop: 'Entfernter Desktop',
      login: 'Einloggen',
      changePassword: 'Passwort ändern',
      terminal: 'Terminal',
      wifi: 'Wi-Fi'
    },
    auth: {
      login: 'Einloggen',
      placeholderUsername: 'Bitte den Benutzernamen eingeben',
      placeholderPassword: 'Bitte das Passwort eingeben',
      placeholderPassword2: 'Passwort wiederholen',
      noEmptyUsername: 'Der Benutzername darf nicht leer sein',
      noEmptyPassword: 'Das Passwort darf nicht leer sein',
      noAccount:
        'Fehler beim Auslsen der Benutzerinformation, bitte die Seite neu laden oder das Passwort zurücksetzen',
      invalidUser: 'Falscher Benutzername oder falsches Passwort',
      error: 'Unerwarteter Fehler',
      changePassword: 'Passwort ändern',
      changePasswordDesc:
        'Für die Sicherheit Ihres Geräts ändern Sie bitte das Web-Login-Passwort.',
      differentPassword: 'Passwörter stimmt nicht überein',
      illegalUsername: 'Benutzername beinhaltet ungültige Zeichen',
      illegalPassword: 'Passwort beinhaltet ungültige Zeichen',
      forgetPassword: 'Passwort vergessen',
      ok: 'Ok',
      cancel: 'Abbrechen',
      loginButtonText: 'Einloggen',
      tips: {
        reset1:
          'To reset the passwords, pressing and holding the BOOT button on the MSPKVM for 10 seconds.',
        reset2: 'For detailed steps, please consult this document:',
        reset3: 'Web default account:',
        reset4: 'SSH default account:',
        change1: 'Please note that this action will change the following passwords:',
        change2: 'Web login password',
        change3: 'System root password (SSH login password)',
        change4: 'To reset the passwords, press and hold the BOOT button on the MSPKVM.'
      }
    },
    wifi: {
      title: 'Wi-Fi',
      description: 'Configure Wi-Fi for MSPKVM',
      success: 'Please check the network status of MSPKVM and visit the new IP address.',
      failed: 'Operation failed, please try again.',
      confirmBtn: 'Ok',
      finishBtn: 'Finished'
    },
    screen: {
      video: 'Videomodus',
      resolution: 'Auflösung',
      auto: 'Automatisch',
      autoTips:
        'Falls es bei bestimmten Auflösungen zu Screen-Tearing oder einem Offset des Mauszeigers kommen sollte, bitte die Auflösung des Remote Hosts anpassen oder den automatischen Modus ausschalten.',
      fps: 'FPS',
      customizeFps: 'Anpassen',
      quality: 'Qualität',
      qualityLossless: 'Verlustfrei',
      qualityHigh: 'Hoch',
      qualityMedium: 'Mittel',
      qualityLow: 'Niedrig',
      frameDetect: 'Frame Detect',
      frameDetectTip:
        'Berechnet den Unterschied zwischen den Einzelbildern. Beendet die Liveübertragung des Videostreams wenn keine Änderungen auf dem Bildschirm des Hosts festgestellt werden kann.',
      resetHdmi: 'Reset HDMI'
    },
    keyboard: {
      paste: 'Einfügen',
      tips: 'Nur Standard-Tastaturbuchstaben und Symbole werden unterstützt.',
      placeholder: 'Bitte eingeben',
      submit: 'Senden',
      virtual: 'Virtuelle Tastatur',
      ctrlaltdel: 'Ctrl+Alt+Del'
    },
    mouse: {
      default: 'Standard Cursor',
      pointer: 'Zeiger Cursor',
      cell: 'Zellen Cursor',
      text: 'Text Cursor',
      grab: 'Greif Cursor',
      hide: 'Cursor ausblenden',
      mode: 'Maus Modus',
      absolute: 'Absoluter Modus',
      relative: 'Relativer Modus',
      requestPointer:
        'Der Relative Modus wird benutzt. Bitte auf den Desktop klicken um einen Mauszeiger anzuzeigen.',
      resetHid: 'Eingeabegeräte zurücksetzen'
    },
    image: {
      title: 'ISO oder Festplatten Abbilder',
      loading: 'Laden...',
      empty: 'Nichts gefunden',
      mountFailed: 'Mounten fehlgeschlagen',
      mountDesc:
        'Bei einigen Systemen muss die virtuelle Festplatte auf dem Remote-Host ausgeworfen werden, bevor das Image gemountet werden kann.',
      tips: {
        title: 'Wie man Abbilder hochlädt',
        usb1: 'Den MSPKVM via USB mit dem Computer verbinden.',
        usb2: 'Stellen sie sicher, das das Virtuelle Festplatte eingebunden ist. (Einstellungen - Virtuelle Festplatte).',
        usb3: 'Öffnen sie die Virtuelle Festplatte auf ihrem Computer und kopieren sie die Abbilddatei in das root-Verzeichnis des USB-Geräts.',
        scp1: 'Stellen sie sicher, das sich der MSPKVM und ihr Computer im gleichen lokalen Netzwerk befinden.',
        scp2: 'Öffnen sie ein Terminal auf ihrem Computer und benutzen sie den SCP-Befehl um die Abbilddatei in das /data Verzeichnis des MSPKVM zu kopieren.',
        scp3: 'Beispiel: scp ihr-abbild-verzeichnis root@ihre-MSPkvm-ip:/data',
        tfCard: 'TF-Karte',
        tf1: 'Diese Mehthode wird auf Linuxsystemen unterstützt',
        tf2: 'Entfernen sie die TF-Karte aus dem MSPKVM (gilt nur für die fertig zusammengebaute Version, bitte das Gehäuse vorher auseinanderbauen).',
        tf3: 'Legen sie die TF-Karte in den Kartenleser ein und verbinden sie diesen mit dem Computer.',
        tf4: 'Kopieren sie die Abbild-Datei in das /data Verzeichnis der TF-Karte.',
        tf5: 'Legen sie die TF-Karte in das MSPKVM ein.'
      }
    },
    script: {
      title: 'Skript',
      upload: 'Hochladen',
      run: 'Ausführen',
      runBackground: 'Im Hintergrund ausführen',
      runFailed: 'Ausführung abgebrochen',
      attention: 'Achtung',
      delDesc: 'Sind sie sicher, dass sie diese Datei löschen möchten?',
      confirm: 'Ja',
      cancel: 'Nein',
      delete: 'Löschen',
      close: 'Schließen'
    },
    terminal: {
      title: 'Terminal',
      mspkvm: 'MSPKVM Terminal',
      serial: 'Terminal über den Seriellen Port',
      serialPort: 'Serieller Port',
      serialPortPlaceholder: 'Serielle Portnummer eingeben',
      baudrate: 'Baudrate',
      confirm: 'Ok'
    },
    wol: {
      title: 'Wake-on-LAN',
      sending: 'Befehl senden...',
      sent: 'Befehl gesendet',
      input: 'Bitte die MAC eingeben',
      ok: 'Ok'
    },
    power: {
      title: 'Einschalten',
      reset: 'Neustart',
      power: 'Einschalten',
      powerShort: 'Einschalten (Kurzes Drücken)',
      powerLong: 'Einschalten (Langes Drücken)'
    },
    settings: {
      title: 'Settings',
      about: {
        title: 'Über MSPKVM',
        information: 'Information',
        ip: 'IP',
        mdns: 'mDNS',
        application: 'Versionsnummer',
        applicationTip: 'MSPKVM web application version',
        image: 'Image Version',
        imageTip: 'MSPKVM system image version',
        deviceKey: 'Geräte-Key',
        community: 'Community'
      },
      appearance: {
        title: 'Appearance',
        display: 'Display',
        language: 'Language',
        menuBar: 'Menu Bar',
        menuBarDesc: 'Display icons in the menu bar'
      },
      device: {
        title: 'Device',
        oled: {
          title: 'OLED',
          description: 'OLED screen automatically sleep',
          0: 'Never',
          15: '15 sec',
          30: '30 sec',
          60: '1 min',
          180: '3 min',
          300: '5 min',
          600: '10 min',
          1800: '30 min',
          3600: '1 hour'
        },
        wifi: {
          title: 'Wi-Fi',
          description: 'Configure Wi-Fi',
          setBtn: 'Config'
        },
        disk: 'Virtual Disk',
        diskDesc: 'Mount virtual U-disk on the remote host',
        network: 'Virtual Network',
        networkDesc: 'Mount virtual network card on the remote host'
      },
      tailscale: {
        title: 'Tailscale',
        memory: {
          title: 'Memory optimization',
          tip: "When memory usage exceeds the limit, garbage collection is performed more aggressively to attempt to free up memory. it's recommended to set to 50MB if using Tailscale. A Tailscale restart is required for the change to take effect.",
          disable: 'Disable'
        },
        restart: 'Are you sure to restart Tailscale?',
        stop: 'Are you sure to stop Tailscale?',
        stopDesc: 'Log out Tailscale and disable its automatic startup on boot.',
        loading: 'Lade...',
        notInstall: 'Tailscale wurde nicht gefunden! Bitte installieren.',
        install: 'Installieren',
        installing: 'Installation wird durchgeführt',
        failed: 'Installation gescheitert',
        retry:
          'Bitte neu laden und noch einmal versuchen. Oder versuchen sie Tailscale manuell zu installieren',
        download: 'Laden sie das',
        package: 'Installationspaket',
        unzip: 'und entpacken sie es manuell',
        upTailscale: 'Hochladen von tailscale in das MSPKVM Verzeichnis /usr/bin/',
        upTailscaled: 'Hochladen von tailscaled in das MSPKVM Verzeichnis /usr/sbin/',
        refresh: 'Die aktuelle Seite neu laden',
        notLogin:
          'Diese Geräte ist bisher noch nicht verknüpft. Bitte loggen sie sich in ihr Konto ein und verknüpfen sie dieses Gerät mit diesem.',
        urlPeriod: 'Diese URL ist für 10 Minuten gültig',
        login: 'Einloggen',
        loginSuccess: 'Das Einloggen war erfolgreich',
        enable: 'Tailscale aktivieren',
        deviceName: 'Name des Geräts',
        deviceIP: 'Geräte-IP',
        account: 'Benutzerkonto',
        logout: 'Ausloggen',
        logout2: 'Wollen Sie sich wirklich ausloggen?',
        okBtn: 'Yes',
        cancelBtn: 'No'
      },
      update: {
        title: 'Nach neuem Update suchen',
        queryFailed: 'Versionsnummer konnte nicht erkannt werden',
        updateFailed: 'Update gescheitert. Bitte versuchen sie es erneut.',
        isLatest: 'Die aktuelle Version ist bereits installiert.',
        available:
          'Ein Update ist verfügbar. Sind sie sicher, dass sie diese Version aktualisieren möchten?',
        updating: 'Update wird gestartet. Bitte warten...',
        confirm: 'Bestätigen',
        cancel: 'Abbrechen'
      },
      account: {
        title: 'Account',
        webAccount: 'Web Account Name',
        password: 'Password',
        updateBtn: 'Update',
        logoutBtn: 'Logout'
      }
    }
  }
};

export default de;
