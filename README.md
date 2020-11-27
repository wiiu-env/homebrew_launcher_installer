# Homebrew Launcher installer
Straight port from the offical [homebrew_launcher repo](https://github.com/dimok789/homebrew_launcher)

# Usage
Meant to be used with the  [payload_loader](https://github.com/wiiu-env/payload_loader).  
Put the created `payload.elf` on the sd card where the `payload_loader` can find it.
Check out the repository of the loader for further instructions.

Still requires the [homebrew_launcher.elf](https://github.com/dimok789/homebrew_launcher)
to be placed on the sd card. Checkout the [official hbl repo](https://github.com/dimok789/homebrew_launcher)
for further instructions.

## Building
In order to be able to compile this, you need to have installed
[devkitPPC](https://devkitpro.org/wiki/Getting_Started) with the following
pacman packages installed.

```
pacman -Syu devkitPPC
```

Make sure the following environment variables are set:
```
DEVKITPRO=/opt/devkitpro
DEVKITPPC=/opt/devkitpro/devkitPPC
```

The command `make` should produce a `payload.elf`, meant to be used with the
[payload_loader](https://github.com/wiiu-env/payload_loader)

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t hbl-installer-builder

# make 
docker run -it --rm -v ${PWD}:/project hbl-installer-builder make

# make clean
docker run -it --rm -v ${PWD}:/project hbl-installer-builder make clean
```


# Credits

- dimok789: [original installer](https://github.com/dimok789/homebrew_launcher))
- orboditilt: port to be used with the [payload_loader](https://github.com/wiiu-env/payload_loader)
