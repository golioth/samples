import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import { IonList, IonItem, IonLabel } from '@ionic/react';

import { useDeviceList } from '../api'
import './Home.css';

const Home: React.FC = () => {
  const { devices } = useDeviceList()
  return (
    <IonPage>
      <IonHeader>
        <IonToolbar>
          <IonTitle>Smart Lamp</IonTitle>
        </IonToolbar>
      </IonHeader>
      <IonContent fullscreen>
        <IonHeader collapse="condense">
          <IonToolbar>
            <IonTitle size="large">Smart Lamp</IonTitle>
          </IonToolbar>
        </IonHeader>
        <IonList>
          {devices.map( device => {
            return (
              <IonItem routerLink={`/devices/${device.id}`}>
                <IonLabel>{device.name}</IonLabel>
              </IonItem>
            )
          })}
        </IonList>
      </IonContent>
    </IonPage>
  );
};

export default Home;
