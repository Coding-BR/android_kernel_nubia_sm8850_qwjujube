# ZTE Charger Policy Binary Evidence

`official_zte_charger_policy.ko` is the stock ARM64 module used as the
reverse-engineering reference for the reconstructed charger policy driver. It
is evidence, not a source file and not a kernel build input.

## Identity

- Original repository location: `/official_zte_charger_policy.ko`
- SHA-256: `0513da230bbc9b82efacdac8309bbc24f94c9ae40f7faefc617ca7e86bf8316c`
- ELF Build ID: `7b723c2d13fb7c9d19e8ae5256e5c5b1ef86fa93`
- Module name: `zte_charger_policy`
- Module license declaration: `GPL v2`
- Vermagic: `6.12.23-maybe-dirty-4k SMP preempt mod_unload modversions aarch64`

Verify the evidence before using it:

```bash
sha256sum -c SHA256SUMS
```

The build-targeted reconstruction lives at
`vendor/qcom/opensource/zte-drivers/zte_charger_policy/`.
