pragma Singleton
import QtQuick

/**
 * @brief Global application state singleton for Neo-Z
 *
 * Provides cross-component communication and shared state.
 *
 * Usage:
 *   AppState.isDeviceConnected = true
 *   AppState.showNotification("Device connected", "success")
 *   AppState.navigateTo(1)
 */
QtObject {
    id: root

    // --- Connection State ---
    property bool isDeviceConnected: false
    property string currentDevice: ""
    property string adbStatus: "Offline"
    property bool freeFireRunning: false

    // --- Navigation State ---
    property int activePage: 0
    property var pageHistory: []

    // --- UI State ---
    property bool sidebarExpanded: true
    property bool isLoading: false
    property string loadingMessage: ""

    // --- Input State ---
    property bool inputHookActive: false
    property bool drcsActive: false

    // --- AI State ---
    property bool aiProcessing: false
    property bool hasRecommendation: false

    // --- Signals for cross-component communication ---
    signal showNotification(string message, string type)  // type: "info", "success", "warning", "error"
    signal navigationRequested(int page)
    signal refreshRequested
    signal deviceConnected(string deviceId)
    signal deviceDisconnected

    // --- Navigation Methods ---
    function navigateTo(page) {
        if (activePage !== page) {
            pageHistory.push(activePage);
            activePage = page;
            navigationRequested(page);
        }
    }

    function goBack() {
        if (pageHistory.length > 0) {
            activePage = pageHistory.pop();
            navigationRequested(activePage);
        }
    }

    // --- State Update Methods ---
    function setConnected(deviceId) {
        currentDevice = deviceId;
        isDeviceConnected = true;
        adbStatus = "Connected";
        deviceConnected(deviceId);
    }

    function setDisconnected() {
        currentDevice = "";
        isDeviceConnected = false;
        adbStatus = "Offline";
        freeFireRunning = false;
        deviceDisconnected();
    }

    function setLoading(loading, message = "") {
        isLoading = loading;
        loadingMessage = message;
    }
}
