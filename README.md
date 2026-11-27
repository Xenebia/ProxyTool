# ProxyTool

## ⚠️ Avertissement : à utiliser à vos propres risques

**Projet personnel. Ne l'utilisez pas si vous ne comprenez pas ce qu'il fait.**

### Ce que fait le programme

- Il modifie le **proxy système de Windows** pour le **compte utilisateur actuel** uniquement
  (clé de registre `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings`,
  valeurs `ProxyEnable` et `ProxyServer`).
- Il n'a pas besoin des droits administrateur.
- Il écrit un journal des actions (démarrage, authentification, activation, désactivation).

### Ce qu'il faut savoir avant de l'utiliser

- L'adresse du proxy est **écrite en dur** dans `config.h` (`PROXY_ADDRESS` / `PROXY_PORT`).
  C'est une adresse de réseau local : si ce proxy n'est pas joignable depuis votre réseau,
  **l'accès à Internet sera coupé** après l'activation. Cliquez alors sur « Désactiver le proxy ».
- Le mot de passe du programme n'est **pas une protection de sécurité** : il est visible dans
  le code source et dans l'exécutable. Il sert uniquement à s'assurer que l'utilisation est
  une **action volontaire**. Une confirmation supplémentaire est demandée avant chaque activation.
- L'auteur n'est pas responsable des modifications de paramètres effectuées avec cet outil :
  c'est l'utilisateur qui décide de saisir le mot de passe et de confirmer l'activation.
- Les antivirus (Windows Defender, etc.) peuvent signaler un exécutable non signé qui modifie
  le registre. Compilez-le vous-même si vous avez un doute.
- N'utilisez cet outil que sur des machines et des réseaux où vous avez le droit de modifier
  les paramètres proxy.

### Annuler les changements

- Via le programme : bouton / option « Désactiver le proxy ».
- Ou manuellement : *Paramètres Windows → Réseau et Internet → Proxy → désactiver « Utiliser un serveur proxy »*.

## Versions

| Dossier | Type |
| --- | --- |
| `nouvelle_version/` | Interface graphique (fenêtre Windows) |
| `ancienne_version/` | Version console (menu texte) |

## Compilation

Prérequis : MinGW-w64 (`g++`) dans le PATH.

1. Ouvrir le dossier de la version voulue.
2. Lancer `build.bat`.

Le script utilise `-static` pour intégrer les bibliothèques de MinGW (`libstdc++`, `libgcc`,
`libwinpthread`) dans `ProxyTool.exe`, afin qu'il fonctionne sur un PC **sans MinGW**.
Sans cette option, l'exécutable affiche une erreur de DLL manquante sur les autres machines.

Pour vérifier :

```
objdump -p ProxyTool.exe | findstr /i "DLL Name"
```

Il ne doit rester que des DLL système Windows (`KERNEL32`, `USER32`, `WININET`, `api-ms-win-crt-*`, ...).

## Journal

- Nouvelle version : `%LOCALAPPDATA%\ProxyTool\audit.log`
- Ancienne version : `C:\ProgramData\ProxyTool\audit.log`
