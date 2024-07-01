# PWM (Password Manager)

This program implements a password manager in the form of a console utility. All data is recorded according to the "key -> value" principle in a binary file of a custom format and is protected using a master password and AES-256 encryption. The OpenSSL library is used to implement encryption.

## Usage example

```shell
$ pwm some-password qwerty123
Enter the password: *********

$ pwm some-password
Enter the password: *********
some-password: qwerty123
```