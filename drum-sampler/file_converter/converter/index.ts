import { W_OK, R_OK } from 'constants'
import fs from 'fs/promises'
import { glob } from 'glob'
import path from 'path'

type InputType = 'rawSigned' | 'rawUnsigned' | 'wav' | 'mp4'
type OutputType = 'separateFilesBundle' | 'separateFiles' | 'singleFile'

type ConverterParams = {
  inputType: InputType
  inputGlob: string
  outputPath: string
  contiguousArray: 'on' | undefined
}

// why do I have to do this...
export const fileExists = (file: string, mode?: number): Promise<boolean> => {
  return fs
    .access(file, mode || W_OK | R_OK)
    .then(() => true)
    .catch(() => false)
}

const signedToUnsigned = (num: number, size = 8) => {}

const hFileTemplate = (name: string) => `#include <stdint.h>
#include <stdlib.h>
#ifndef _${name.toLocaleUpperCase()}_H
#define _${name.toLocaleUpperCase()}_H

extern const size_t ${name.toLocaleUpperCase()}_SIZES[];
extern const size_t ${name.toLocaleUpperCase()}_OFFSETS[];
extern const uint8_t ${name.toLocaleUpperCase()}[];

#endif`
const cFileTemplate = (name: string, sizes: string, offsets: string, samples: string) => `#include <stdint.h>
#include <stdlib.h>
#include "${name}.h"

const size_t ${name.toLocaleUpperCase()}_SIZES[] = ${sizes};
const size_t ${name.toLocaleUpperCase()}_OFFSETS[] = ${offsets};

const uint8_t ${name.toLocaleUpperCase()}[] = ${samples};`

export const convert = async ({ inputType, inputGlob, outputPath, contiguousArray }: ConverterParams) => {
  console.log('formData', { inputType, inputGlob, outputPath, contiguousArray })
  const inputFiles = await glob(inputGlob, {
    cwd: '..',
    nodir: true,
  })
  console.log('inputFiles', inputFiles)
  if (!['rawSigned', 'rawUnsigned'].includes(inputType))
    throw new Error(`Input Type: ${inputType} is not yet supported`)

  const outputPathFromRoot = path.join('..', outputPath)
  const { name: filename, dir: dirname } = path.parse(outputPathFromRoot)

  // write h file
  fs.writeFile(`${dirname}/${filename}.h`, hFileTemplate(filename))

  // write c file
  let sizes = '{\n'
  let offsets = '{\n'
  let currentOffset = 0
  let samples = '{\n'
  for (const [i, file] of inputFiles.entries()) {
    const _buf = await fs.readFile(path.join('..', file))
    const buffer = inputType === 'rawUnsigned' ? _buf : new Int8Array(_buf)
    if (i != 0) sizes += ',\n'
    if (i != 0) offsets += ',\n'
    if (i != 0) samples += ',\n'

    const length = buffer.length
    sizes += `  ${length}`
    offsets += `  ${currentOffset}`
    currentOffset += length
    samples += `  // ${i}: ${path.basename(file)}`
    if (!contiguousArray) samples += '\n  {'
    for (const [j, byte] of buffer.entries()) {
      if (j != 0) samples += ','
      if (!(j % 16)) samples += '\n  '
      if (!(j % 16) && !contiguousArray) samples += '  '
      samples += byte > 15 ? '0x' : '0x0'
      samples += byte.toString(16)
    }
    if (!contiguousArray) samples += '\n  }'
  }
  sizes += '\n}'
  offsets += '\n}'
  samples += '\n}'

  fs.writeFile(`${dirname}/${filename}.c`, cFileTemplate(filename, sizes, offsets, samples))

  console.log(`Converted ${inputFiles.length} file${inputFiles.length > 1 ? 's' : ''}`)

  return { convertedCount: inputFiles.length }
}
