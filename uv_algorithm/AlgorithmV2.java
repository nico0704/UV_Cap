import java.util.Scanner;
import java.lang.Math;

public class AlgorithmV2 {
    public static Scanner in = new Scanner(System.in);
    public static double currentMinutes;  // -> Anzahl der Minuten, die man schon in der Sonne verbracht hat (Abgezogen vom currentProtectionTimer)
    public static double maxMinutes; // -> Gesamtzeit, die man in der Sonne verbringen kann (protectedMinutes)
    public static int maxIndex; // höchster gemessener UV Index (0-11)
    public static final double meds[] = {150.0, 250.0, 300.0, 450.0, 600.0, 900.0};
    public static double med;
    public static double lsf;

    public static void main(String[] args) {
        setup();
        loop();
    }

    public static void setup() {
        currentMinutes = 0;
        maxMinutes = 1;
        maxIndex = 0;
        med = meds[(int) getSkinType() - 1];
        lsf = getLSF();
    }

    public static void loop() {
        while (true) {
            if (currentMinutes > 0) {
                // an App senden
                System.out.println("Verbleibende Zeit bis zum Sonnenbrand: " + (maxMinutes - currentMinutes));
            } 
            if (currentMinutes >= maxMinutes) {
                System.out.println("Aus der Sonne du Knecht!!!");
                break;
            }
            double uv_index = getUVIndex();
            if (uv_index > 2) {
                currentMinutes++;
            }
            if ((int) uv_index <= maxIndex) {
                continue;
            }
            // Werte v
            if (maxIndex == 0) {
                System.out.println("maxIndex == 0 -> Werte werden initial berechnet.");
                double protectedMinutes = calcMinutes(med, uv_index, lsf);
                maxMinutes = Math.round(protectedMinutes);
                continue;
            }
            double newProtectedMinutes = calcMinutes(med, uv_index, lsf);
            currentMinutes = Math.round((currentMinutes / maxMinutes) * newProtectedMinutes);
            maxMinutes = Math.round(newProtectedMinutes);
        }
    }

    public static double calcMinutes(double med, double uv_index, double lsf) {
        double protectedMinutes = calcProtectedMinutes(lsf, calcSelfProtectionTime(med, uv_index));
        maxIndex = (int) uv_index;
        return protectedMinutes;
    }

    public static double getUVIndex() {
        // Idee: nicht nur einmal den UV Index pro Minute holen, sondern für 5 - 10 Sekunden Werte holen, speichern und dann den Mittelwert nehmen
        // um Sprünge / Messfehler zu vermeiden
        System.out.println("Enter UV_Index:");
        double uv_index = in.nextInt();
        return uv_index;
    }

    public static double getLSF() {
        // 1 - 50
        System.out.println("Enter LSF:");
        double lsf = in.nextInt();
        if (lsf <= 0) {
            lsf = 1;
        }
        return lsf;
    }

    public static double calcSelfProtectionTime(double med, double uv_index) {
        return med / (uv_index * 1.5);
    }

    public static double calcProtectedMinutes(double lsf, double selfProtectionTime) {
        return selfProtectionTime * lsf * 0.6;
    }

    public static double getSkinType() {
        // 1 - 6
        System.out.println("Enter Skin Type:");
        double skinType = in.nextInt();
        return skinType;
    }
}
