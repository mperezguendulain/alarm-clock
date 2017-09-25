/*
* Autores:  Martín Alejandro Pérez Güendulain
*           Ángel Roberto Móntez Murgas
*/

let app = new Vue({
    el: '#app',
    data: {
        horas: 12,
        minutos: 0,
        segundos: 0,
        ampm: "am",
        socket: null
    },
    methods: {
        convertToTimeFormat: function(num) {
            if(num < 10)
                return "0"+num;
            return num.toString();
        },
        incHoras: function() {
            if(this.horas == 12)
                this.horas = 1;
            else
                this.horas++;
        },
        decHoras: function() {
            if(this.horas == 0)
                this.horas = 12;
            else
                this.horas--;
        },
        incMinutos: function() {
            if(this.minutos == 59)
                this.minutos = 0;
            else
                this.minutos++;
        },
        decMinutos: function() {
            if(this.minutos == 0)
                this.minutos = 59;
            else
                this.minutos--;
        },
        incSegundos: function() {
            if(this.segundos == 59)
                this.segundos = 0;
            else
                this.segundos++;
        },
        decSegundos: function() {
            if(this.segundos == 0)
                this.segundos = 59;
            else
                this.segundos--;
        },
        toggleAMPM: function() {
            if(this.ampm == "am")
                this.ampm = "pm";
            else
                this.ampm = "am";
        },
        setTime: function() {
            this.socket.emit('setTime', this.convertToTimeFormat(this.horas) + ":" +
                                        this.convertToTimeFormat(this.minutos) + ":" +
                                        this.convertToTimeFormat(this.segundos) +
                                        this.ampm);
            alert("Hora seteada Exitosamente");
        }
    },
    beforeMount: function() {
        this.socket = io();
    }
});
