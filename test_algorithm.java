import java.util.Scanner;

public class test_algorithm {
    static Scanner in = new Scanner(System.in);
    //static highest_UV_factor
    //static currentMinutes -> Anzahl der Minuten, die man schon in der Sonne verbracht hat (Abgezogen vom currentProtectionTimer)
    //static currentProtectionTimer -> Gesamtzeit, die man in der Sonne verbringen kann (protectedMinutes)
    public static void main(String[] args) {
        double meds[] = {150.0, 250.0, 300.0, 450.0, 600.0, 900.0};
        double skinType = getSkinType();
        int medsIndex = (int) skinType - 1;
        double med = meds[medsIndex];
        System.out.println(med);
        double lsf = getLSF();
        //Loop (jede Minute 1x, wenn Akkulaufzeit zu gering hochgehen)
        double uv_index = getUVIndex();
        //Abgleich, ob UV-Index höher als vorher
        //Regelmäßig nach UV-Faktor fragen, wenn UV-Wert höher als highest_UV_factor
        //  wenn UV-Index niedriger als 2 -> Ziehe keine Minuten vom Timer ab
        double selfProtectionTime = calcSelfProtectionTime(med, uv_index);
        System.out.println("Eigenschutzzeit : " + selfProtectionTime);
        double protectedMinutes = calcProtectedMinutes(lsf, selfProtectionTime);
        System.out.println("Schutzzeit in Minuten: " + protectedMinutes);
        //  dann:   Prozentzahl = vergangene Minuten / Anzahl-Gesamtminuten
        //          neue Minutenzahl = calcProctectedMinutes(lsf, selfProtectionTime -> mit neuem UV-Wert) * Prozentzahl
    }

    public static double getUVIndex() {
        System.out.println("Enter UV_Index:");
        double uv_index = in.nextInt();
        return uv_index;
    }

    public static double getLSF() {
        System.out.println("Enter LSF:");
        double lsf = in.nextInt();
        return lsf;
    }

    public static double calcSelfProtectionTime(double med, double uv_index) {
        return med / (uv_index * 1.5);
    }

    public static double calcProtectedMinutes(double lsf, double selfProtectionTime) {
        return (selfProtectionTime * lsf) * 0.6;
    }

    public static double getSkinType() {
        System.out.println("Enter Skin Type:");
        double skinType = in.nextInt();
        return skinType;
    }
}
