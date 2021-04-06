import {
  useEffect,
  useState,
} from 'react'

const API_KEY = "YOUR_API_KEY"
const PROJECT_ID = "YOUR_PROJECT_ID"

const BASE_URL = "https://api.golioth.dev/v1"

interface Device {
  id: string;
  name: string;
}

export const setDeviceData = async (deviceId: string, path: string, data: any) => {
  const url = `${BASE_URL}/projects/${PROJECT_ID}/devices/${deviceId}/data/${path}`
  try {
    const res = await fetch(url, {
      method: "PUT",
      body: JSON.stringify(data),
      headers: {
        "X-API-KEY" : API_KEY
      }
    })
    const json = await res.json()
    return json
  }catch(err){
    return null
  }
  return null
}

interface Data {
  [key: string]: any;
}

export const useDeviceData = (deviceId: string, path: string) => {
  const [data, setData] = useState<Data>({})
  const [loading, setLoading] = useState(true)
  const url = `${BASE_URL}/projects/${PROJECT_ID}/devices/${deviceId}/data/${path}`
  const refresh = async () => {
    try {
      const res = await fetch(url, {
        headers: {
          "X-API-KEY" : API_KEY
        }
      })
      const json = await res.json()
      setData(json.data)
    }catch(err){
      setData({})
    }finally{
      setLoading(false)
    }

  }

  useEffect( () => {
    refresh()
  }, [])

  return { data, loading, refresh }
}

export const useDevice = (id: string) => {
  const [device, setDevice] = useState<Device | null>(null)
  const [loading, setLoading] = useState(true)

  useEffect( () => {
    const url = `${BASE_URL}/projects/${PROJECT_ID}/devices/${id}`

    async function getDevice(){
      try {
        const res = await fetch(url, {
          headers: {
            "X-API-KEY" : API_KEY
          }
        })
        const json = await res.json()
        setDevice(json.data)
      }catch(err){
        setDevice(null)
      }finally{
        setLoading(false)
      }

    }
    getDevice()
  }, [])

  return { device, loading }
}

export const useDeviceList = () => {
  const [devices, setDevices] = useState<Array<Device>>([])
  const [loading, setLoading] = useState(true)

  useEffect( () => {
    const url = `${BASE_URL}/projects/${PROJECT_ID}/devices`

    async function getDevices(){
      try {
        const res = await fetch(url, {
          headers: {
            "X-API-KEY" : API_KEY
          }
        })
        const json = await res.json()
        setDevices(json.list)
      }catch(err){
        setDevices([])
      }finally{
        setLoading(false)
      }

    }
    getDevices()
  }, [])

  return { devices, loading }
}
