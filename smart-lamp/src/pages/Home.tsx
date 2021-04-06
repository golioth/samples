import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import { IonList, IonItem, IonLabel, IonInput, IonToggle, IonRadio, IonCheckbox, IonItemSliding, IonItemOption, IonItemOptions } from '@ionic/react';

import ExploreContainer from '../components/ExploreContainer';
import { useDeviceList } from '../api'
import './Home.css';

const Home: React.FC = () => {
  const { devices, loading } = useDeviceList()
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
