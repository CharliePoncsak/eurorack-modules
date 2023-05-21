// Next.js API route support: https://nextjs.org/docs/api-routes/introduction
import { convert } from '@/converter'
import type { NextApiRequest, NextApiResponse } from 'next'

type Response = {
  converted: boolean
}

export default async function handler(req: NextApiRequest, res: NextApiResponse<Response>) {
  const info = await convert(req.body)
  res.status(200).json(info)
}
