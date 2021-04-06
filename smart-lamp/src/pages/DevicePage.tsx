import { IonBackButton, IonButtons, IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import { IonList, IonItem, IonLabel, IonInput, IonToggle, IonRadio, IonCheckbox, IonItemSliding, IonItemOption, IonItemOptions } from '@ionic/react';
import { RouteComponentProps } from 'react-router';

import { useDeviceData, useDevice, setDeviceData } from '../api'
import './Home.css';

interface DevicePageProps extends RouteComponentProps<{
  deviceId: string;
}> {}

const DevicePage: React.FC<DevicePageProps> = ({ match }) => {
  const deviceId = match.params.deviceId
  const { data: ledData, refresh } = useDeviceData(deviceId, "led")
  const { device } = useDevice(deviceId)
  return (
    <IonPage>
      <IonContent>
        <IonHeader>
          <IonToolbar>
            <IonButtons slot="start">
              <IonBackButton defaultHref="/home" />
            </IonButtons>
            <IonTitle>{device && device.name}</IonTitle>
          </IonToolbar>
        </IonHeader>
        <IonList>
          {Object.keys(ledData || {}).map( key => {
            return (
              <IonItem>
                <IonLabel>LED {key}</IonLabel>
                <IonToggle checked={ledData[key]} onIonChange={ async (e) => {
                  await setDeviceData(deviceId,`/led/${key}`, !ledData[key])
                  refresh()
                }}/>
              </IonItem>
            )
          })}
        </IonList>
      </IonContent>
    </IonPage>
  );
};

export default DevicePage;
