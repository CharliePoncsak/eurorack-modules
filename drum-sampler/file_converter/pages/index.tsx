import Head from 'next/head'
import styles from '@/styles/Home.module.scss'
import { useState } from 'react'

const onFormSubmit = async (formData: any) => {
  console.log(formData)
  const response = await fetch('api/convert', {
    method: 'POST',
    body: JSON.stringify(formData),
    headers: {
      'content-type': 'application/json',
    },
  })

  const json = await response.json()
  console.log('Converter returned: ', json)
  return json
}

export default function Home() {
  const [convertedCount, setConvertedCount] = useState(0)
  return (
    <>
      <Head>
        <title>File Converter</title>
        <meta name='description' content='File converter for drum sampler' />
        <meta name='viewport' content='width=device-width, initial-scale=1' />
        <link rel='icon' href='/favicon.ico' />
      </Head>
      <main className={styles.main}>
        <div className={styles.description}>Convert files to C language data format</div>
        {/* <code className={styles.code}>pages/index.tsx</code> */}
        <form
          className={styles.settingsForm}
          onSubmit={e => {
            e.preventDefault()
            const formData = Object.fromEntries(new FormData(e.target as HTMLFormElement).entries())
            onFormSubmit(formData).then(json => {
              if (json?.convertedCount && !isNaN(json?.convertedCount)) setConvertedCount(json?.convertedCount)
            })
          }}
        >
          <h1>Input</h1>
          <fieldset className={styles.fieldset}>
            <legend>Input file type:</legend>
            <div>
              <input defaultChecked type='radio' name='inputType' id='inputTypeRawUnsigned' value='rawUnsigned' />
              <label htmlFor='rawUnsigned'>Raw Unsigned</label>
            </div>
            <div>
              <input type='radio' name='inputType' id='inputTypeRawSigned' value='rawSigned' />
              <label htmlFor='rawSigned'>Raw Signed</label>
            </div>
            <div>
              <input disabled type='radio' name='inputType' id='inputTypeWav' value='wav' />
              <label className={styles.textGray} htmlFor='wav'>
                .wav
              </label>
            </div>
            <div>
              <input disabled type='radio' name='inputType' id='inputTypeMp4' value='mp4' />
              <label className={styles.textGray} htmlFor='wav'>
                .mp4
              </label>
            </div>
          </fieldset>
          <fieldset className={styles.fieldset}>
            <legend>Input glob pattern</legend>
            <input type='text' name='inputGlob' id='' defaultValue={'./Samples/**/*.raw'} />
          </fieldset>
          <h1>Output</h1>
          <fieldset className={styles.fieldset}>
            <legend>Output type:</legend>
            {/* <div>
              <input type='radio' name='outputType' id='outputTypeSingleFile' value='singleFile' />
              <label htmlFor='outputTypeSingleFile'>Single file</label>
            </div>
            <div>
              <input disabled type='radio' name='outputType' id='outputTypeSeparateFiles' value='separateFiles' />
              <label className={styles.textGray} htmlFor='outputTypeSeparateFiles'>
                Separate files
              </label>
            </div> */}
            <div>
              <input defaultChecked type='checkbox' id='contiguousArray' name='contiguousArray' />
              <label htmlFor='contiguousArray'>Use a contiguous (1D) array (recommended)</label>
            </div>
          </fieldset>
          <fieldset className={styles.fieldset}>
            <legend>Output file name</legend>
            <input type='text' name='outputPath' defaultValue={'audio_data'} />
          </fieldset>
          <input type='submit' value='Convert' />
          {convertedCount ? <span className={styles.greenText}>Converted {convertedCount} files</span> : ''}
        </form>
      </main>
    </>
  )
}
