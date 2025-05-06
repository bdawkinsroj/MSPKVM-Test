const it = {
  translation: {
    head: {
      desktop: 'Desktop Remoto',
      login: 'Accesso',
      changePassword: 'Cambia Password',
      terminal: 'Terminale',
      wifi: 'Wi-Fi'
    },
    auth: {
      login: 'Accesso',
      placeholderUsername: 'Inserisci il nome utente',
      placeholderPassword: 'Inserisci la password',
      placeholderPassword2: 'Inserisci nuovamente la password',
      noEmptyUsername: 'Il nome utente non può essere vuoto',
      noEmptyPassword: 'La password non può essere vuota',
      noAccount:
        'Impossibile ottenere informazioni utente, aggiorna la pagina o reimposta la password',
      invalidUser: 'Nome utente o password non validi',
      error: 'Errore imprevisto',
      changePassword: 'Cambia Password',
      changePasswordDesc:
        'Per la sicurezza del tuo dispositivo, modifica la password di accesso web.',
      differentPassword: 'Le password non corrispondono',
      illegalUsername: 'Il nome utente contiene caratteri non validi',
      illegalPassword: 'La password contiene caratteri non validi',
      forgetPassword: 'Hai dimenticato la password',
      ok: 'Ok',
      cancel: 'Annulla',
      loginButtonText: 'Accedi',
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
      video: 'Modalità video',
      resolution: 'Risoluzione',
      auto: 'Automatico',
      autoTips:
        'Potrebbero verificarsi tearing dello schermo o spostamento del mouse a risoluzioni specifiche. Considera di regolare la risoluzione del dispositivo remoto o disabilitare la modalità automatica.',
      fps: 'FPS',
      customizeFps: 'Personalizza',
      quality: 'Qualità',
      qualityLossless: 'Senza perdita',
      qualityHigh: 'Alto',
      qualityMedium: 'Medio',
      qualityLow: 'Basso',
      frameDetect: 'Rilevamento Frame',
      frameDetectTip:
        'Calcola la differenza tra i frame. Interrompe la trasmissione del flusso video quando non vengono rilevate modifiche sullo schermo del dispositivo remoto.',
      resetHdmi: 'Reset HDMI'
    },
    keyboard: {
      paste: 'Incolla',
      tips: 'Sono supportati solo lettere e simboli standard della tastiera',
      placeholder: 'Inserisci testo',
      submit: 'Invia',
      virtual: 'Tastiera',
      ctrlaltdel: 'Ctrl+Alt+Del'
    },
    mouse: {
      default: 'Cursore predefinito',
      pointer: 'Cursore a puntatore',
      cell: 'Cursore a cella',
      text: 'Cursore testo',
      grab: 'Cursore di presa',
      hide: 'Nascondi cursore',
      mode: 'Modalità mouse',
      absolute: 'Modalità assoluta',
      relative: 'Modalità relativa',
      requestPointer:
        'Usando la modalità relativa. Clicca sul desktop per ottenere il puntatore del mouse.',
      resetHid: 'Reimposta HID'
    },
    image: {
      title: 'Immagini',
      loading: 'Caricamento...',
      empty: 'Nessun risultato',
      mountFailed: 'Montaggio immagine fallito',
      mountDesc:
        "In alcuni sistemi, è necessario espellere il disco virtuale sull'host remoto prima di montare l'immagine.",
      tips: {
        title: 'Come caricare',
        usb1: 'Collega il MSPKVM al tuo computer tramite USB.',
        usb2: 'Assicurati che la Virtual Disk sia montata (Impostazioni - Virtual Disk).',
        usb3: 'Apri il disk virtuale sul tuo computer e copia il file immagine nella directory principale del disk.',
        scp1: 'Assicurati che il MSPKVM e il tuo computer siano sulla stessa rete locale.',
        scp2: 'Apri un terminale sul tuo computer e usa il comando SCP per caricare il file immagine nella directory /data del MSPKVM.',
        scp3: 'Esempio: scp il-tuo-percorso-immagine root@il-tuo-ip-mspkvm:/data',
        tfCard: 'Scheda TF',
        tf1: 'Questo metodo è supportato su sistemi Linux',
        tf2: 'Recupera la scheda TF dal MSPKVM (per la versione FULL, smonta prima il case).',
        tf3: 'Inserisci la scheda TF in un lettore di schede e collegala al tuo computer.',
        tf4: 'Copia il file immagine nella directory /data sulla scheda TF.',
        tf5: 'Inserisci la scheda TF nel MSPKVM.'
      }
    },
    script: {
      title: 'Script',
      upload: 'Carica',
      run: 'Esegui',
      runBackground: 'Esegui in Background',
      runFailed: 'Esecuzione fallita',
      attention: 'Attenzione',
      delDesc: 'Sei sicuro di voler eliminare questo file?',
      confirm: 'Sì',
      cancel: 'No',
      delete: 'Elimina',
      close: 'Chiudi'
    },
    terminal: {
      title: 'Terminale',
      mspkvm: 'Terminale MSPKVM',
      serial: 'Terminale Porta Seriale',
      serialPort: 'Porta Seriale',
      serialPortPlaceholder: 'Inserisci la porta seriale',
      baudrate: 'Baud rate',
      confirm: 'Ok'
    },
    wol: {
      title: 'Wake-on-LAN',
      sending: 'Invio comando...',
      sent: 'Comando inviato',
      input: 'Inserisci il MAC',
      ok: 'Ok'
    },
    power: {
      title: 'Accensione',
      reset: 'Reimposta',
      power: 'Accensione',
      powerShort: 'Accensione (clic breve)',
      powerLong: 'Accensione (clic lungo)'
    },
    settings: {
      title: 'Settings',
      about: {
        title: 'Informazioni su MSPKVM',
        information: 'Informazioni',
        ip: 'IP',
        mdns: 'mDNS',
        application: 'Versione Applicazione',
        applicationTip: 'MSPKVM web application version',
        image: 'Versione Immagine',
        imageTip: 'MSPKVM system image version',
        deviceKey: 'Chiave Dispositivo',
        community: 'Comunità'
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
        loading: 'Caricamento...',
        notInstall: 'Tailscale non trovato! Per favore, installa.',
        install: 'Installa',
        installing: 'Installazione in corso',
        failed: 'Installazione fallita',
        retry: 'Riprova aggiornando la pagina o installa manualmente',
        download: 'Scarica il',
        package: 'pacchetto di installazione',
        unzip: 'e decomprimilo',
        upTailscale: 'Carica tailscale nella directory /usr/bin/ del MSPKVM',
        upTailscaled: 'Carica tailscaled nella directory /usr/sbin/ del MSPKVM',
        refresh: 'Aggiorna la pagina corrente',
        notLogin:
          'Il dispositivo non è ancora stato associato. Effettua il login e associa questo dispositivo al tuo account.',
        urlPeriod: 'Questo URL è valido per 10 minuti',
        login: 'Accedi',
        loginSuccess: 'Accesso riuscito',
        enable: 'Abilita Tailscale',
        deviceName: 'Nome Dispositivo',
        deviceIP: 'IP Dispositivo',
        account: 'Account',
        logout: 'Disconnetti',
        logout2: 'Sei sicuro di voler uscire?',
        okBtn: 'Yes',
        cancelBtn: 'No'
      },
      update: {
        title: 'Controlla Aggiornamenti',
        queryFailed: 'Impossibile ottenere la versione',
        updateFailed: 'Aggiornamento fallito. Riprova.',
        isLatest: 'Hai già la versione più recente.',
        available: 'Un aggiornamento è disponibile. Sei sicuro di voler aggiornare?',
        updating: 'Aggiornamento avviato. Attendere prego...',
        confirm: 'Conferma',
        cancel: 'Annulla'
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

export default it;
