# HashCheck

A simple precommitment authentication scheme

## What is it?

HashCheck is a simple tool designed to hash random data over and over. While that seems rather pointless, it offers a powerful way to repetitively, and iteratively prove an identity. The hash function is SHA256, but will hopefully become independent of any specific algorithm in the future.

## Usage

The following will create a pair of keys, as with traditional cryptography, keep your private (secret) key secret! 

```bash
> ./hashcheck -c           
Generating pair with 4194304 iterations
Private:
	6ca1a10d082647f49ab7bdc91b05287a35390704af39f660792ee1c735e1a502
Public:
	16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7

> ./hashcheck -c > secret.txt && chmod 600 secret.txt
Generating pair with 4194304 iterations
```
After publishing your public key somewhere, you can verify you own it with:

```bash
> ./hashcheck -p
Please enter your private key: 6ca1a10d082647f49ab7bdc91b05287a35390704af39f660792ee1c735e1a502
Please enter known public key: 16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7
Verifying: (public)
	16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7
With: (private)
	dd8396393530293297d935efe41331063ed200ebbd6e42949df99694855f4328
Success!
```

By revealing the key `dd8396393530293297d935efe41331063ed200ebbd6e42949df99694855f4328` you can verify that you produced the original public key. For a person to verify such claims, run the following:

```bash
> ./hashchech -v
Please enter known public key: 16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7
Please enter verification key: dd8396393530293297d935efe41331063ed200ebbd6e42949df99694855f4328
Verifying: (public)
	16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7
With: (private)
	dd8396393530293297d935efe41331063ed200ebbd6e42949df99694855f4328
Success, 1 iterations!
```

or the following one liner:

```bash
> echo -n dd8396393530293297d935efe41331063ed200ebbd6e42949df99694855f4328 | xxd -r -p | sha256sum
16566c9f1a684cf1a663a729a0ed295a0e59fa2c01d92d0a1a1130d471f3e2e7  -
```

## Disclaimer

This tool has no guarantee of safety, in fact, by itself it is very dangerous and you may end up killing someone with it. Be careful. 
