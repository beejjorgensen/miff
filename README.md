# Sample File Archive

Dirt simple non-compressing archiver for demonstration purposes.

Though archivers like ZIP are rather complex, at their core it's pretty
simple: pack all the files into a single file with just enough metadata
to extract them later.

## Shortcomings

* Much error checking missing.
* Non-portable out of Unix land.
* Doesn't handle subdirectories.
* Can't yet extract. Whatever.

Again, just a demo.

## Spec

The archive is made of sequential chunks of data.

Each chunk has a header consisting of the file name and file size,
followed by the raw file data.

```
+---------------------+--------+-------------
|     file name       +  size  |  data  ...
+---------------------+--------+-------------
```

* file name: 256 bytes, trailing zero padding
* size: 8 byte, unsigned 64-bit value, big-endian
* data: raw file data

## Usage

* `miff a file.miff file1 file2`: create archive `file.miff`
  containing `file1` and `file2`.

* `miff l file.miff`: list the contents of the archive

* `miff x file.miff`: extract files from the archive (unimplemented)

